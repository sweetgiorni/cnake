from dataclasses import dataclass


@dataclass
class Project:
    """CMake project"""
    name: str


@dataclass
class Target:
    """CMake target"""
    name: str
