#include <iostream>
#include <locale>
#include <iomanip>
#include <windows.h>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>

#pragma comment( lib, "setupapi.lib" )

using namespace std;

struct Device {
	std::string vendorId;
	std::string deviceId;
	std::string deviceInfo;
	std::string vendorInfo;
};

class DevicesDatabase {
public:
	std::vector<std::string> data;
	void Initialize();
};

class Parser
{
public:
	DevicesDatabase db;

	std::string getVendorId(std::string info);
	std::string getDeviceId(std::string info);
	std::string getVendorName(std::string vendorId);
	std::string getDeviceName(std::string deviceId);

	Device Parse(std::string info);

	Parser();
	~Parser();
};

class PciDevicesProvider
{
private:
	SP_DEVINFO_DATA DevInfoData;
	HDEVINFO DevInfoSet;

	char pbuffer[512];

	Parser parser;

public:
	bool Initialize();
	std::vector<Device> GetDevices();
	void DestoyInfoList();
};

int main() {
	PciDevicesProvider provider;
	provider.Initialize();

	vector<Device> Device = provider.GetDevices();

	cout << "PCI Devices: " << Device.size() << endl;

	for (int i = 0; i < Device.size(); i++) {

		cout << "VendorId:	" << Device.at(i).vendorId
			<< "		DeviceId:	" << Device.at(i).deviceId
			<< endl;
	}
	provider.DestoyInfoList();
	cout << endl;
	system("pause");
	return EXIT_SUCCESS;
}

/////////////////////

bool PciDevicesProvider::Initialize() {
	DevInfoSet = SetupDiGetClassDevs(
		NULL,								// Class GUID
		"PCI",								// Enumerator
		NULL,								// hwndParent
		DIGCF_ALLCLASSES | DIGCF_PRESENT);	// Flags

	if (DevInfoSet == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	else return TRUE;
}

std::vector<Device> PciDevicesProvider::GetDevices() {
	DevInfoData.cbSize = sizeof(DevInfoData);

	std::vector<Device> devicesList;

	int count = 0;
	while (SetupDiEnumDeviceInfo(DevInfoSet, count, &DevInfoData)) {
		SetupDiGetDeviceRegistryProperty(
			DevInfoSet,					// Device Info Set
			&DevInfoData,				// Device Info Data
			SPDRP_HARDWAREID,			// Propetry	
			NULL,						// Property Reg Data Type
			(BYTE*)pbuffer,				// Property Buffer
			512,						// Property Buffer Size 
			NULL);						// Required Size

		Device device = parser.Parse(std::string(pbuffer));
		devicesList.push_back(device);

		count++;
	}

	return devicesList;
}

void PciDevicesProvider::DestoyInfoList() {
	SetupDiDestroyDeviceInfoList(DevInfoSet);
}

/////////////////

std::string Parser::getVendorId(std::string info) {
	std::regex regVENID = std::regex(std::string("VEN_.{4}"));
	std::smatch match;
	std::regex_search(info, match, regVENID);

	std::string id = std::string(match[0]).substr(4, 4);

	std::transform(id.begin(), id.end(), id.begin(), ::tolower);

	return id;
}

std::string Parser::getDeviceId(std::string info) {
	std::regex regDEVID = std::regex(std::string("DEV_.{4}"));
	std::smatch match;
	std::regex_search(info, match, regDEVID);

	std::string id = std::string(match[0]).substr(4, 4);

	std::transform(id.begin(), id.end(), id.begin(), ::tolower);

	return id;
}

std::string Parser::getVendorName(std::string vendorId) {
	std::regex regVEN = std::regex(std::string("^" + vendorId + "  "));

	for (int i = 0; i < db.data.size(); i++) {
		std::smatch match;
		if (std::regex_search(db.data.at(i), match, regVEN)) {
			return match.suffix();
		}
	}

	return "";
}

std::string Parser::getDeviceName(std::string deviceId) {
	std::regex regDEV = std::regex(std::string("^\\t" + deviceId + "  "));

	for (int i = 0; i < db.data.size(); i++) {
		std::smatch match;
		if (std::regex_search(db.data.at(i), match, regDEV)) {
			return match.suffix();
		}
	}

	return "";
}

Device Parser::Parse(std::string info) {
	Device device;

	device.deviceId = getDeviceId(info);
	device.vendorId = getVendorId(info);

	device.deviceInfo = getDeviceName(device.deviceId);
	device.vendorInfo = getVendorName(device.vendorId);

	return device;
}

Parser::Parser() {
	db.Initialize();
}

Parser::~Parser() {
}


void DevicesDatabase::Initialize() {

}