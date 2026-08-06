"""Import-safety tests for local analysis scripts."""

import builtins
import importlib.util
import unittest
from unittest.mock import patch


class AnalysisImportTests(unittest.TestCase):
    """Verify imports do not cross executable boundaries."""

    def test_analysis_scripts_import_without_running_commands(self) -> None:
        """Importing either script needs no ML, plotting, hosted, or process work."""
        removed_modules = {"comet_ml", "matplotlib", "numpy", "tensorflow", "torch", "torchvision"}
        real_import = builtins.__import__

        def guarded_import(
            name: str,
            global_namespace: dict[str, object] | None = None,
            local_namespace: dict[str, object] | None = None,
            fromlist: tuple[str, ...] = (),
            level: int = 0,
        ) -> object:
            if name.partition(".")[0] in removed_modules:
                message = f"removed dependency imported: {name}"
                raise AssertionError(message)
            return real_import(name, global_namespace, local_namespace, fromlist, level)

        with patch("builtins.__import__", side_effect=guarded_import), patch("subprocess.Popen") as popen:
            for module_name in ("scripts.compare_implementations", "scripts.optimize_initialize"):
                spec = importlib.util.find_spec(module_name)
                if spec is None or spec.loader is None:
                    self.fail(f"could not find an import loader for {module_name}")
                module = importlib.util.module_from_spec(spec)
                spec.loader.exec_module(module)
                self.assertTrue(callable(module.main))

        popen.assert_not_called()


if __name__ == "__main__":
    unittest.main()
