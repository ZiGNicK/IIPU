#include <iostream>
#include <iomanip>
#include <Windows.h>
#include <WinIoCtl.h>
#include <ntddscsi.h>
#include <conio.h>

#include "Standarts.h"

using namespace std;


void ShowInfo(DiskDrive drive) {
	cout << drive.ProductId << endl;			// Model
	cout << drive.Revision << endl;				//Version
	cout << drive.BusType << endl;				//Bus type
	cout << drive.SerialNumber << endl << endl;	//Serial number

	cout << drive.memory.totalDiskSpace << endl;	//Total space[Mb]
	cout << drive.memory.totalFreeSpace << endl;	//Free space[Mb]
	cout << 100.0 - (double)drive.memory.totalFreeSpace / (double)drive.memory.totalDiskSpace * 100 << endl;	//Busy space[%]

	cout << endl;

	//ATA Support
	for (int i = 8; i >= 4; i--) {
		if (drive.standarts.ata.support[i] == 1)
			cout << "ATA" << i << " , ";
	}
	cout << endl;

	//DMA Support
	for (int i = 0; i < 8; i++) {
		if (drive.standarts.dma.support[i] == 1)
			cout << "DMA" << i << " , ";
	}
	cout << endl;

	//PIO Support
	for (int i = 0; i < 2; i++) {
		if (drive.standarts.pio.support[i] == 1)
			cout << "PIO" << i + 3 << " , ";
	}
	cout << endl << endl;;
}

int main() {
	StorageInfoProvider provider;
	DiskDrive hdd = provider.getHddInfo();

	ShowInfo(hdd);

	system("pause");
	return 0;
}