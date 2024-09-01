from abc import ABC, abstractmethod

class Singleton(ABC):
    _instance = None

    def __new__(cls, *args, **kwargs):
        if cls._instance is None:
            cls._instance = super(Singleton, cls).__new__(cls, *args, **kwargs)
        return cls._instance

    @abstractmethod
    def initialize(self):
        pass
