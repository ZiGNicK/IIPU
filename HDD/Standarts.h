#pragma once

class Interface {
public:
	int support[16];

	virtual void setSupported(unsigned short supported) = 0;
};

class ATA : public Interface {
public:
	void setSupported(unsigned short supported) override {
		int i = 16;

		while (i--) {
			support[i] = supported & 32768 ? 1 : 0;
			supported <<= 1;
		}
	}
};

class DMA : public Interface {
public:
	void setSupported(unsigned short supported) override {
		int i = 16;

		while (i--) {
			support[i] = supported & 32768 ? 1 : 0;
			supported <<= 1;
		}
	}
};

class PIO : public Interface {
public:
	void setSupported(unsigned short supported) override {
		int i = 16;

		while (i--) {
			support[i] = supported & 32768 ? 1 : 0;
			supported <<= 1;
		}
	}
};

class Standarts {
public:
	ATA ata;
	DMA dma;
	PIO pio;

	void getSupportedInterfaces(HANDLE diskHandle) {
		UCHAR identifyDataBuffer[512 + sizeof(ATA_PASS_THROUGH_EX)] = { 0 };

		ATA_PASS_THROUGH_EX &PTE = *(ATA_PASS_THROUGH_EX *)identifyDataBuffer;
		PTE.Length = sizeof(PTE);
		PTE.TimeOutValue = 10;
		PTE.DataTransferLength = 512;
		PTE.DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX);
		PTE.AtaFlags = ATA_FLAGS_DATA_IN;

		IDEREGS *ideRegs = (IDEREGS *)PTE.CurrentTaskFile;
		ideRegs->bCommandReg = 0xEC;
		DeviceIoControl(
			diskHandle,
			IOCTL_ATA_PASS_THROUGH,
			&PTE,
			sizeof(identifyDataBuffer),
			&PTE,
			sizeof(identifyDataBuffer),
			NULL,
			NULL
		);

		WORD *data = (WORD *)(identifyDataBuffer + sizeof(ATA_PASS_THROUGH_EX));

		ata.setSupported(data[80]);
		dma.setSupported(data[63]);
		pio.setSupported(data[63]);
	}
};

#define MEGABYTE 1048576

class Memory {
public:
	int totalDiskSpace;
	int totalFreeSpace;

	Memory() {
		_ULARGE_INTEGER totalDiskSpace, diskSpace;
		_ULARGE_INTEGER totalFreeSpace, freeSpace;

		totalDiskSpace.QuadPart = totalFreeSpace.QuadPart = 0;

		unsigned long int logicalDrivesCount = GetLogicalDrives();

		for (char var = 'A'; var < 'Z'; var++) {
			if ((logicalDrivesCount >> var - 65) & 1) {
				std::string path;
				path = var;
				path.append(":\\");

				if (GetDriveType(path.c_str()) == DRIVE_FIXED) {
					GetDiskFreeSpaceEx(path.c_str(), 0, &diskSpace, &freeSpace);

					totalDiskSpace.QuadPart += diskSpace.QuadPart / MEGABYTE;
					totalFreeSpace.QuadPart += freeSpace.QuadPart / MEGABYTE;
				}
			}
		}

		this->totalDiskSpace = totalDiskSpace.QuadPart;
		this->totalFreeSpace = totalFreeSpace.QuadPart;
	}
};

class DiskDrive
{
public:
	char* ProductId;
	char* Revision;
	char* BusType;
	char* SerialNumber;

	Standarts standarts;

	Memory memory;
};

const char* busType[] = { "UNKNOWN", "SCSI", "ATAPI", "ATA", "ONE_TREE_NINE_FOUR", "SSA", "FIBRE", "USB", "RAID", "ISCSI", "SAS", "SATA", "SD", "MMC" };

class StorageInfoProvider {
	DiskDrive drive;
	HANDLE diskHandle;

public:
	StorageInfoProvider() {
		diskHandle = CreateFile(
			"\\\\.\\PhysicalDrive0",		// File name
			GENERIC_READ | GENERIC_WRITE,	// Access
			FILE_SHARE_READ,				// Security attributes
			NULL,
			OPEN_EXISTING,
			NULL,
			NULL
		);
	}

	DiskDrive getHddInfo() {
		STORAGE_PROPERTY_QUERY storagePropertyQuery;
		storagePropertyQuery.QueryType = PropertyStandardQuery;
		storagePropertyQuery.PropertyId = StorageDeviceProperty;

		STORAGE_DEVICE_DESCRIPTOR* deviceDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)calloc(1024, 1);

		if (!DeviceIoControl(
			diskHandle,						// hDevice
			IOCTL_STORAGE_QUERY_PROPERTY,	// dwloControlCode
			&storagePropertyQuery,			// lplnBuffer 
			sizeof(storagePropertyQuery),	// nlnBufferSize
			deviceDescriptor,				// lpOutBuffer
			1024,							// nOutBufferSize
			NULL,							// lpBytesReturned
			NULL							// lpOverlapper
		)) {
			printf("%d", GetLastError());
			CloseHandle(diskHandle);
			exit(-1);
		}

		drive.ProductId = (char*)(deviceDescriptor)+deviceDescriptor->ProductIdOffset;
		drive.Revision = (char*)(deviceDescriptor)+deviceDescriptor->ProductRevisionOffset;
		drive.BusType = (char*)busType[deviceDescriptor->BusType];
		drive.SerialNumber = (char*)(deviceDescriptor)+deviceDescriptor->SerialNumberOffset;

		drive.standarts.getSupportedInterfaces(diskHandle);

		return drive;
	}
};
