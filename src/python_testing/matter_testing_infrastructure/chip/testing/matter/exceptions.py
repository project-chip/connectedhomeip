"""Custom exceptions for Matter testing infrastructure."""


class MatterTestError(Exception):
    """Base class for all Matter testing exceptions."""
    pass


class CommissioningError(MatterTestError):
    """Raised when there is an error during device commissioning."""
    pass


class TestConfigurationError(MatterTestError):
    """Raised when there is an error in test configuration."""
    pass


class EndpointError(MatterTestError):
    """Raised when there is an error related to endpoints."""
    pass


class AttributeError(MatterTestError):
    """Raised when there is an error related to attributes."""
    pass


class CommandError(MatterTestError):
    """Raised when there is an error related to commands."""
    pass


class FeatureError(MatterTestError):
    """Raised when there is an error related to features."""
    pass


class TimeoutError(MatterTestError):
    """Raised when an operation times out."""
    pass
