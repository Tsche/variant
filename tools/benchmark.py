import os
import logging
import json
import zlib
from pathlib import Path
from typing import Any, Optional
from subprocess import check_call
from collections import defaultdict

import itertools

import jmespath

from palgen import Extension, Sources, Model
from palgen.ingest import Name, Toml

class Config(Model):
    source: Path
    profile: dict[str, Any]
    query: dict[str, Any]
    standard: Optional[str] = None
    options: list[str] = []
    true_value: str = "1"
    false_value: str = "0"



def dict_product(dicts):
    return (dict(zip(dicts, x)) for x in itertools.product(*dicts.values()))

def generate_trace(file, output, defines: dict[str, Any], config: Config):
    defines = [f'-D{name}={value}' for name, value in defines.items()]
    includes = ['-isystem', str(Path.cwd() / 'include')]
    standard = f"-std={config.standard}" if config.standard else '-std=c++23'
    profile_flags = ['-ftime-trace', '-c']

    taskset_prefix = ["taskset", "--cpu-list", "0"]
    call = [*taskset_prefix, os.environ.get("CXX") or "clang++", str(file), '-o', str(output), *
            defines, *includes, standard, *profile_flags, *config.options]

    output.parent.mkdir(exist_ok=True, parents=True)
    check_call(call, cwd=file.parent)
    output.unlink()

    trace_file = output.with_suffix('.json')
    logging.info("Generated %s", trace_file)
    return trace_file


def analyze_trace(file: Path, queries):
    trace_times = defaultdict(list)
    trace = json.loads(file.read_text('utf-8'))
    for query_name, entry in queries.items():
        search_result = jmespath.search(entry, trace)

        if len(search_result) > 1:
            logging.warning("Found more than one result")
            continue

        trace_times[query_name].append(search_result[0])
    return trace_times

class Analyze:
    def __init__(self, config: Config, repeat_each: int, file: Path, output: Path):
        self.config = config
        self.output = output
        self.repeat_each = repeat_each
        self.queries = {name: jmespath.compile(
            query) for name, query in config.query.items()}
        self.config.profile = dict(self.expand_profiles(self.config.profile.copy()))
        self.fix_bools()
        self.runs: dict[str, list[Path]] = {
            name: [generate_trace(file, self.output / file.parent.stem / f'{name}.{idx}.obj', features, config)
                   for idx in range(self.repeat_each)]
            for name, features in self.config.profile.items()}

    def fix_bools(self):
        for name, features in self.config.profile.items():
            for feature, value in features.copy().items():
                if isinstance(value, bool):
                    features[feature] = self.config.true_value if value else self.config.false_value

    def get_ranges(self, features: dict):
        for feature, value in features.items():
            if isinstance(value, dict):
                assert "min" in value and "max" in value, "Invalid range: Missing min or max"
                yield feature, range(int(value["min"]), int(value["max"]) + 1)
            elif isinstance(value, list):
                yield feature, value

    def expand_profiles(self, profile_input: dict[str, Any]):
        for name, features in profile_input.items():
            ranges = dict(self.get_ranges(features))
            if not ranges:
                yield name, features

            for feature_set in dict_product(ranges):
                new_name = f"{name}_{'_'.join(f'{feature}{value}' for feature, value in feature_set.items())}"
                values = features.copy()
                for feature, value in feature_set.items():
                    values[feature] = value
                yield new_name, values

    def _search(self, trace_file: Path):
        trace = json.loads(trace_file.read_text('utf-8'))
        for name, query in self.queries.items():
            result = query.search(trace)
            if len(result) > 1:
                logging.warning("More than one result found")
                continue

            yield name, result[0]

    def run_queries(self):
        feature_sets = {name: [dict(self._search(file))
                               for file in files]
                        for name, files in self.runs.items()}
        return {query: {name: [run[query]
                               for run in runs]
                        for name, runs in feature_sets.items()}
                for query in self.queries}

    def worst_offenders(self):
        expensive_classes = jmespath.compile("reverse(sort_by(traceEvents[?name=='ParseClass' "
                                             "&& !starts_with(@.args.detail, 'std')], &dur)[])[:10]")
        expensive_functions = jmespath.compile("reverse(sort_by(traceEvents[?name=='InstantiateFunction' "
                                               "&& !starts_with(@.args.detail, 'std')], &dur)[])[:10]")


class Benchmark(Extension):
    ingest = Sources >> Name("profiles.toml") >> Toml

    class Settings(Model):
        output: Path
        repeat_each: int = 1
        system_includes: list[str] = []
        includes: list[str] = []

    Schema = Config

    def run_benchmark(self, data):
        if not self.settings.output.is_absolute():
            self.settings.output = self.root_path / self.settings.output

        for path, config in data:
            if not (file := Path(config.source)).is_absolute():
                file = path.parent / file
            analyzer = Analyze(config, self.settings.repeat_each, file, self.settings.output)
            queries = analyzer.run_queries()
            colors = [f"#{hex(zlib.crc32(name.encode('utf-8')))[4:]}"
                      for name in config.profile]
            summary = {}
            for idx, (query, runs) in enumerate(queries.items()):
                datasets = []
                yMarkers = []
                chart_data = {"labels": [str(idx) for idx in range(self.settings.repeat_each)],
                              "datasets": datasets,
                              "yMarkers": yMarkers}
                logging.info("Query %s", query)
                summary[query] = {}
                for name, run in runs.items():
                    summary[query][name] = min(run)
                    datasets.append({"name": name,
                                     "values": run,
                                     "chartType": 'line'})
                    yMarkers.append({"label": name, "value": min(run)})
                    logging.info("  Feature set %s", name)
                    logging.info("    Results: %s", run)
                    logging.info("    Avg: %sms", (sum(run) / len(run)) / 1000)
                    logging.info("    Min: %sms", min(run) / 1000)
                chart = self.settings.output / path.parent.name / f"{query}.json"

                yield chart, json.dumps({'title': query,
                                         'data': chart_data,
                                         'type': "line",
                                         'height': 400,
                                         'colors': colors,
                                         'lineOptions': {'regionFill': 1, 'spline': 1}})
            summary_file = self.settings.output / path.parent.name / 'summary.json'
            yield summary_file, json.dumps(summary)

    pipeline = ingest >> Extension.validate >> run_benchmark >> Extension.write
