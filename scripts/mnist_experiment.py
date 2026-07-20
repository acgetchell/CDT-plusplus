"""Run the CDT++ TensorFlow MNIST experiment."""

from __future__ import annotations

import argparse
import sys
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from collections.abc import Sequence


def _parse_args(argv: Sequence[str]) -> argparse.Namespace:
    """Parse command-line arguments without loading TensorFlow."""
    parser = argparse.ArgumentParser(description=__doc__)
    return parser.parse_args(argv)


def _run_experiment() -> None:
    """Train and evaluate the historical TensorFlow model."""
    import tensorflow as tf  # noqa: PLC0415

    tf.keras.utils.set_random_seed(0)
    mnist = tf.keras.datasets.mnist
    (x_train, y_train), (x_test, y_test) = mnist.load_data()
    x_train, x_test = x_train / 255.0, x_test / 255.0

    model = tf.keras.models.Sequential(
        [
            tf.keras.layers.Flatten(input_shape=(28, 28)),
            tf.keras.layers.Dense(128, activation="relu"),
            tf.keras.layers.Dropout(0.2),
            tf.keras.layers.Dense(10),
        ]
    )
    predictions = model(x_train[:1]).numpy()
    tf.nn.softmax(predictions).numpy()

    loss_function = tf.keras.losses.SparseCategoricalCrossentropy(from_logits=True)
    loss_function(y_train[:1], predictions).numpy()
    model.compile(optimizer="adam", loss=loss_function, metrics=["accuracy"])
    model.fit(x_train, y_train, epochs=5)
    model.evaluate(x_test, y_test, verbose=2)

    probability_model = tf.keras.Sequential([model, tf.keras.layers.Softmax()])
    probability_model(x_test[:5])


def main(argv: Sequence[str] | None = None) -> int:
    """Run the MNIST experiment from an installed uv entry point."""
    _parse_args(sys.argv[1:] if argv is None else argv)
    try:
        _run_experiment()
    except ModuleNotFoundError as error:
        print(
            f"Missing experiment dependency {error.name!r}; run with `uv run --group experiments cdt-mnist-experiment`.",
            file=sys.stderr,
        )
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
