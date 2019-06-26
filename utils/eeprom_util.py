import re
import sys
import bisect
import struct
from enum import Enum


class RecordType(Enum):
    DATA = 0
    END_OF_FILE = 1
    EXTENDED_SEGMENT_ADDRESS = 2
    START_SEGMENT_ADDRESS = 3
    EXTENDED_LINEAR_ADDRESS = 4
    START_LINEAR_ADDRESS = 5


class IHexRecord(object):

    IHEX_FORMAT = re.compile(r'^:[0-9a-fA-F]+$')

    def __init__(self, address, _type, data):
        self.data = data
        self.address = address
        self.type = _type

    def __str__(self):
        data_str = ' '.join('0x%02x' % b for b in self.data)

        return '<%s %d bytes at 0x%04x = %s>' % (
            self.__class__.__name__, self.length, self.address, data_str)

    def __repr__(self):
        return str(self)

    @property
    def length(self):
        return len(self.data)

    @classmethod
    def parse_string(cls, line):
        if len(line) < 11:
            raise RuntimeError('IHex: invalid record format (record must be 11 chars or more)')

        if not cls.IHEX_FORMAT.match(line):
            raise RuntimeError('IHex: invalid record format (unexpected characters: %r)' % line)

        line = line[1:]
        length = int(line[0:2], 16)

        if len(line) != 10 + length * 2:
            raise RuntimeError('IHex: invalid record length')

        address = int(line[2:6], 16)
        _type = RecordType(int(line[6:8], 16))
        data = bytearray.fromhex(line[8:8 + length * 2])
        return cls(address, _type, data)

    def has_address(self, address):
        return address >= self.address and address < self.address + self.length

    def get_data(self, address, length):
        start = address - self.address
        return self.data[start:start + length]


class IHex(object):

    def __init__(self, records):
        self.records = sorted((r for r in records if r.type == RecordType.DATA), lambda x, i: x.address)
        self._optimize()
        self.addresses = [r.address for r in self.records]

    def _optimize(self):
        if len(self.records) == 0:
            return

        last = self.records[0]
        result = [last]

        for record in self.records[1:]:
            if record.address < last.address + last.length + 1024:
                padding_length = record.address - (last.address + last.length)
                last.data += b'\x00' * padding_length + record.data
            else:
                last = record
                result.append(last)

        self.records = result

    @classmethod
    def parse_string(cls, s):
        records = [IHexRecord.parse_string(line) for line in s.splitlines()]
        return cls(records)

    @classmethod
    def parse_file(cls, path):
        with open(path, 'r') as f:
            records = [IHexRecord.parse_string(line.strip()) for line in f.readlines()]
        return cls(records)

    def get_record_for_address(self, address):
        index = bisect.bisect_right(self.addresses, address) - 1

        if index == -1 or not self.records[index].has_address(address):
            return None

        return self.records[index]

    def get_data(self, address, length):
        result = bytes()
        current_address = address
        remaining_length = length

        while remaining_length > 0:
            record = self.get_record_for_address(current_address)

            if record is None:
                raise RuntimeError('No data for address 0x%04x' % current_address)

            to_append = min(record.length - (current_address - record.address), remaining_length)
            result += record.get_data(current_address, to_append)
            current_address += to_append
            remaining_length -= to_append

        return result

class Calibration(object):

    OFFSET = 0x80
    SIZE = 0x0a

    REF_VOLTAGE = 3.3

    def __init__(self, needle_zero, needle_ref, peak_zero, peak_ref, is_valid):
        self.needle_zero = needle_zero
        self.needle_ref = needle_ref
        self.peak_zero = peak_zero
        self.peak_ref = peak_ref
        self.is_valid = is_valid

    @classmethod
    def to_v(cls, x):
        return cls.REF_VOLTAGE * x / 1024.0

    def __str__(self):
        if self.is_valid == 0xdead:
            is_valid_str = "(valid)"
        else:
            is_valid_str = "(invalid)"

        return "needle: %d/%d %0.2fV+%0.2fV; peak: %d/%d %0.2fV+%0.2fV %s" % (
            self.needle_ref, self.needle_zero,
            self.to_v(self.needle_zero), self.to_v(self.needle_ref - self.needle_zero),
            self.peak_ref, self.peak_zero,
            self.to_v(self.peak_zero), self.to_v(self.peak_ref - self.peak_zero),
            is_valid_str)

    def __repr__(self):
        return "<%s %s>" % (self.__class__.__name__, str(self))

    @classmethod
    def parse_binary_data(cls, buf):
        needle_zero, needle_ref, peak_zero, peak_ref, is_valid = struct.unpack('<HHHHH', buf)
        return cls(needle_zero, needle_ref, peak_zero, peak_ref, is_valid)


class Fault(object):

    OFFSET = 0
    SIZE = 32 + 2 + 1

    def __init__(self, code, extended_status, message):
        self.code = code
        self.extended_status = extended_status
        self.message = message

    @classmethod
    def parse_binary_data(cls, buf):
        fmt = '<BH32s'
        code, extended_status, message = struct.unpack(fmt, buf)
        return cls(code, extended_status, message)


data = sys.stdin.read()
eeprom = IHex.parse_string(data)

fault_data = eeprom.get_data(Fault.OFFSET, Fault.SIZE)
fault = Fault.parse_binary_data(fault_data)

calibration_l_data = eeprom.get_data(Calibration.OFFSET, Calibration.SIZE)
calibration_r_data = eeprom.get_data(Calibration.OFFSET + Calibration.SIZE, Calibration.SIZE)

calibration_l = Calibration.parse_binary_data(calibration_l_data)
calibration_r = Calibration.parse_binary_data(calibration_r_data)

print("")
print("Calibration data:")
print("    Left:  %s" % calibration_l)
print("   Right:  %s" % calibration_r)
print("")
print("Fault data:")
print("   Code: 0x%02x (%d)" % (fault.code, fault.code))
print("   Extended Status: 0x%04x (%d)" % (fault.extended_status, fault.extended_status))
print("   Message: %r (%d)" % (fault.message, len(fault.message)))
print("")
