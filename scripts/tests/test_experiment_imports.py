"""Import-safety tests for optional experiment scripts."""

from __future__ import annotations

import builtins
import importlib.util
import unittest
from unittest.mock import patch


class ExperimentImportTests(unittest.TestCase):
    """Verify imports do not cross optional or executable boundaries."""

    def test_experiment_scripts_import_without_running_experiments(self) -> None:
        """Importing either script needs no ML, plotting, Comet, or subprocess work."""
        optional_modules = {"comet_ml", "matplotlib", "numpy", "tensorflow"}
        real_import = builtins.__import__

        def guarded_import(
            name: str,
            global_namespace: dict[str, object] | None = None,
            local_namespace: dict[str, object] | None = None,
            fromlist: tuple[str, ...] = (),
            level: int = 0,
        ) -> object:
            if name.partition(".")[0] in optional_modules:
                message = f"optional dependency imported: {name}"
                raise AssertionError(message)
            return real_import(name, global_namespace, local_namespace, fromlist, level)

        with patch("builtins.__import__", side_effect=guarded_import), patch("subprocess.Popen") as popen:
            for module_name in ("scripts.optimize_initialize", "scripts.mnist_experiment"):
                spec = importlib.util.find_spec(module_name)
                if spec is None or spec.loader is None:
                    self.fail(f"could not find an import loader for {module_name}")
                module = importlib.util.module_from_spec(spec)
                spec.loader.exec_module(module)
                self.assertTrue(callable(module.main))

        popen.assert_not_called()


if __name__ == "__main__":
    unittest.main()
