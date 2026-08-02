"""Synthetic-data determinism test for the PyTorch training loop."""

import unittest
from typing import Any

from scripts.mnist_experiment import _build_model, _evaluate, _train_epoch


class MnistTrainingTests(unittest.TestCase):
    """Exercise the CPU baseline without downloading MNIST."""

    @staticmethod
    def _train_once() -> tuple[float, float, float, dict[str, Any]]:
        import torch  # noqa: PLC0415

        torch.manual_seed(7)
        torch.use_deterministic_algorithms(mode=True)
        inputs = torch.arange(16 * 28 * 28, dtype=torch.float32).reshape(16, 1, 28, 28) / 1000.0
        labels = torch.arange(16) % 10
        dataset = torch.utils.data.TensorDataset(inputs, labels)
        generator = torch.Generator().manual_seed(7)
        training_loader = torch.utils.data.DataLoader(dataset, batch_size=4, generator=generator, num_workers=0, shuffle=True)
        test_loader = torch.utils.data.DataLoader(dataset, batch_size=4, num_workers=0, shuffle=False)
        model = _build_model(torch)
        loss_function = torch.nn.CrossEntropyLoss()
        optimizer = torch.optim.Adam(model.parameters(), lr=0.001)
        training_loss = _train_epoch(model, training_loader, loss_function, optimizer)
        test_loss, accuracy = _evaluate(torch, model, test_loader, loss_function)
        return training_loss, test_loss, accuracy, model.state_dict()

    def test_training_is_replayable_on_synthetic_cpu_data(self) -> None:
        """The same seed and local data produce identical metrics and weights."""
        import torch  # noqa: PLC0415

        first = self._train_once()
        second = self._train_once()
        self.assertEqual(first[:3], second[:3])
        self.assertEqual(first[3].keys(), second[3].keys())
        for name in first[3]:
            with self.subTest(parameter=name):
                self.assertTrue(torch.equal(first[3][name], second[3][name]))


if __name__ == "__main__":
    unittest.main()
