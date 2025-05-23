#include "pch.h"
#include "kmbox.h"
#include <iostream>

namespace kmbox
{
	HANDLE serial_handle = nullptr;

	bool connected = false;

	int clamp(int i)
	{
		if (i > 127)
			i = 127;
		if (i < -128)
			i = -128;

		return i;
	}

	std::string find_port(const std::string& targetDescription)
	{
		HDEVINFO hDevInfo = SetupDiGetClassDevsA(&GUID_DEVCLASS_PORTS, 0, 0, DIGCF_PRESENT);
		if (hDevInfo == INVALID_HANDLE_VALUE) return "";

		SP_DEVINFO_DATA deviceInfoData;
		deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

		for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &deviceInfoData); ++i)
		{
			char buf[512];
			DWORD nSize = 0;

			if (SetupDiGetDeviceRegistryPropertyA(hDevInfo, &deviceInfoData, SPDRP_FRIENDLYNAME, NULL, (PBYTE)buf, sizeof(buf), &nSize) && nSize > 0)
			{
				buf[nSize] = '\0';
				std::string deviceDescription = buf;

				size_t comPos = deviceDescription.find("COM");
				size_t endPos = deviceDescription.find(")", comPos);

				if (comPos != std::string::npos && endPos != std::string::npos && deviceDescription.find(targetDescription) != std::string::npos)
				{
					SetupDiDestroyDeviceInfoList(hDevInfo);
					return deviceDescription.substr(comPos, endPos - comPos);
				}
			}
		}
		SetupDiDestroyDeviceInfoList(hDevInfo);
		return "";
	}

	void SendCommand(const std::string& command)
	{
		DWORD bytesWritten;
		if (!WriteFile(serial_handle, command.c_str(), command.length(), &bytesWritten, NULL))
		{
			LOG_ERROR("Failed to write to serial port!");
		}
	}

	void KmboxInitialize(std::string port)
	{

		connected = false;
		if (serial_handle)
		{
			CloseHandle(serial_handle);
			serial_handle = NULL;
		}
		//wprintf(L"Connecting to KMBOX on port %ls\n",port.c_str());
		//std::string str = std::string(port.begin(), port.end());
		port = "\\\\.\\" + find_port("USB-SERIAL CH340");
		LOG_INFO("Connecting to KMBOX on port %s", port.c_str());

		serial_handle = CreateFileA(port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

		if (serial_handle == INVALID_HANDLE_VALUE)
		{
			// print the serialhandle
			LOG_ERROR("Serial handle: %d", serial_handle);
			LOG_ERROR("Failed to open serial port!");
			return;

		}

		if (!SetupComm(serial_handle, 8192, 8192))
		{
			LOG_ERROR("Failed to setup serial port!");
			CloseHandle(serial_handle);
			return;
		}

		COMMTIMEOUTS timeouts = { 0 };
		if (!GetCommTimeouts(serial_handle, &timeouts))
		{
			CloseHandle(serial_handle);
			return;
		}
		timeouts.ReadIntervalTimeout = 0xFFFFFFFF;
		timeouts.ReadTotalTimeoutMultiplier = 0;
		timeouts.ReadTotalTimeoutConstant = 0;
		timeouts.WriteTotalTimeoutMultiplier = 0;
		timeouts.WriteTotalTimeoutConstant = 2000;

		if (!SetCommTimeouts(serial_handle, &timeouts))
		{
			
			CloseHandle(serial_handle);
			return;
		}

		PurgeComm(serial_handle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

		DCB dcbSerialParams = { 0 };
		dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

		if (!GetCommState(serial_handle, &dcbSerialParams))
		{
			LOG_ERROR("Failed to get serial state!");
			CloseHandle(serial_handle);
			return;
		}

		int baud = 115200;
		dcbSerialParams.BaudRate = baud;
		dcbSerialParams.ByteSize = 8;
		dcbSerialParams.StopBits = ONESTOPBIT;
		dcbSerialParams.Parity = NOPARITY;

		if (!SetCommState(serial_handle, &dcbSerialParams))
		{
			LOG_ERROR("Failed to set serial parameters!");
			CloseHandle(serial_handle);
			return;
		}

		LOG_INFO("Connected to KMBOX on port %s", std::string(port).c_str());
		SendCommand("def move_with_check(x,y):\r\n  if not km.side1()==0:\r\n    km.move(x,y,1)\r\n\r\n\r\n\r\n\r\n\r\n");
		connected = true;
	}

	void move(int x, int y)
	{
		if (!connected)
		{
			//LOG_ERROR("not connected?");
			return;
		}
		x = clamp(x);
		y = clamp(y);
		std::string command = "move_with_check(" + std::to_string(x) + "," + std::to_string(y) + ")\r\n";
		SendCommand(command.c_str());
	}

	void left_click()
	{
		if (!connected)
		{
			//LOG_ERROR("not connected?");
			return;
		}
		std::string command = "km.left(1)\r\n";
		SendCommand(command.c_str());
	}

	void left_click_release()
	{
		if (!connected)
		{
			//LOG_ERROR("not connected?");
			return;
		}
		std::string command = "km.left(0)\r\n";
		SendCommand(command.c_str());
	}

	bool IsDown(int virtual_key)
	{
		if (!connected)
			return false;
		return false;
		std::string command = virtual_key == 3 ? "km.middle()\r\n" : "km.side" + std::to_string(virtual_key) + std::string("()\r\n");
		SendCommand(command);
		char readBuffer[256];
		DWORD bytesRead;
		if (!ReadFile(serial_handle, readBuffer, sizeof(readBuffer) - 1, &bytesRead, NULL))
		{
			std::cerr << "Failed to read from serial port!" << std::endl;
			return false;
		}

		if (bytesRead > 0)
		{
			readBuffer[bytesRead] = '\0';
			std::string command2 = virtual_key == 3 ? "km.middle()\r\n1" : "km.side" + std::to_string(virtual_key) + std::string("()\r\n1");
			if (strstr(readBuffer, command2.c_str()))
				return true;
		}
		return false;
	}
}
