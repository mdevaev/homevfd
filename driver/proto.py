# ========================================================================== #
#                                                                            #
#    KVMD - The main PiKVM daemon.                                           #
#                                                                            #
#    Copyright (C) 2018-2024  Maxim Devaev <mdevaev@gmail.com>               #
#                                                                            #
#    This program is free software: you can redistribute it and/or modify    #
#    it under the terms of the GNU General Public License as published by    #
#    the Free Software Foundation, either version 3 of the License, or       #
#    (at your option) any later version.                                     #
#                                                                            #
#    This program is distributed in the hope that it will be useful,         #
#    but WITHOUT ANY WARRANTY; without even the implied warranty of          #
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           #
#    GNU General Public License for more details.                            #
#                                                                            #
#    You should have received a copy of the GNU General Public License       #
#    along with this program.  If not, see <https://www.gnu.org/licenses/>.  #
#                                                                            #
# ========================================================================== #


import struct
import dataclasses

from typing import Optional


# =====
class Packable:
    def pack(self) -> bytes:
        raise NotImplementedError()


class Unpackable:
    @classmethod
    def unpack(cls, data: bytes, offset: int=0) -> "Unpackable":
        raise NotImplementedError()


# =====
@dataclasses.dataclass(frozen=True)
class Header(Packable, Unpackable):
    proto:   int
    rid:     int
    op:      int

    NAK              = 0
    ACK              = 1
    BOOTLOADER       = 2
    REBOOT           = 3
    SET_GRID_STATUS  = 5
    SET_BAT_STATUS   = 6
    SET_BAT_LEVEL    = 7
    SET_WATER_STATUS = 8
    SET_WATER_LEVEL  = 9
    SET_PIXMAP       = 10

    __struct = struct.Struct("<BHB")

    SIZE = __struct.size

    def pack(self) -> bytes:
        return self.__struct.pack(self.proto, self.rid, self.op)

    @classmethod
    def unpack(cls, data: bytes, offset: int=0) -> "Header":
        return Header(*cls.__struct.unpack_from(data, offset=offset))


@dataclasses.dataclass(frozen=True)
class Nak(Unpackable):
    reason: int

    INVALID_COMMAND = 0

    __struct = struct.Struct("<B")

    @classmethod
    def unpack(cls, data: bytes, offset: int=0) -> "Nak":
        return Nak(*cls.__struct.unpack_from(data, offset=offset))


@dataclasses.dataclass(frozen=True)
class Ack(Unpackable):
    @classmethod
    def unpack(cls, data: bytes, offset: int=0) -> "Ack":
        return Ack()


# =====
@dataclasses.dataclass(frozen=True)
class BodySetGridStatus(Packable):
    status: int

    def pack(self) -> bytes:
        return self.status.to_bytes()


@dataclasses.dataclass(frozen=True)
class BodySetBatStatus(Packable):
    status: int

    def pack(self) -> bytes:
        return self.status.to_bytes()


@dataclasses.dataclass(frozen=True)
class BodySetWaterStatus(Packable):
    status: int

    def pack(self) -> bytes:
        return self.status.to_bytes()


@dataclasses.dataclass(frozen=True)
class BodySetBatLevel(Packable):
    level: int

    def __post_init__(self) -> None:
        assert 0 <= self.level <= 4

    def pack(self) -> bytes:
        return self.level.to_bytes()


@dataclasses.dataclass(frozen=True)
class BodySetWaterLevel(Packable):
    level: int

    def __post_init__(self) -> None:
        assert 0 <= self.level <= 8

    def pack(self) -> bytes:
        return self.level.to_bytes()


@dataclasses.dataclass(frozen=True)
class BodySetPixmap(Packable):
    data: bytes

    def __post_init__(self) -> None:
        assert len(self.data) == 420

    def pack(self) -> bytes:
        return self.data


# =====
@dataclasses.dataclass(frozen=True)
class Request:
    header: Header
    body:   (Packable | None) = dataclasses.field(default=None)

    def pack(self) -> bytes:
        msg = self.header.pack()
        if self.body is not None:
            msg += self.body.pack()
        return msg


@dataclasses.dataclass(frozen=True)
class Response:
    header: Header
    body:   Unpackable

    @classmethod
    def unpack(cls, msg: bytes) -> Optional["Response"]:
        header = Header.unpack(msg)
        match header.op:
            case Header.NAK:
                return Response(header, Nak.unpack(msg, Header.SIZE))
            case Header.ACK:
                return Response(header, Ack.unpack(msg, Header.SIZE))
        # raise RuntimeError(f"Unknown OP in the header: {header!r}")
        return None
