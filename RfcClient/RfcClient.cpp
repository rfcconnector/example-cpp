// RfcClient.cpp : a simple client with Rfc Connector

#include "stdafx.h"
#include <iostream>
using namespace std;

// import Rfc Connector TLB
#import "libid:FBDBDBA5-518B-4146-A52F-455C0E03492D" no_namespace

void leave(int code)
{
	system("PAUSE");
	CoUninitialize();
	exit(code);
}

void logon(IRfcSessionPtr &session) {
	session->RfcSystemData->ConnectString = L"SAPLOGON_ID=NPL";
	session->LogonData->Client = L"001";
	session->LogonData->User = L"DEVELOPER";
	session->LogonData->Password = L"developer1";
	session->LogonData->Language = L"EN";

	HRESULT hr = session->Connect();
	if (hr != S_OK || session->Error)
	{
		cout << "Error: could not connect to SAP system\n";
		if (session->Error)
			cout << session->ErrorInfo->Message << "\n";
		else
			cout << "hr=0x" << hex << hr << "\n";

		leave(-1);
	}
}

void call_function(IRfcSessionPtr &session) {
	// import the function call prototype
	IFunctionCallPtr fn = session->ImportCall(L"BAPI_FLIGHT_GETLIST", VARIANT_TRUE);

	// set IMPORTING parameters
	fn->Importing->Item[L"AIRLINE"]->value = L"LH";

	// call the function
	// NOTE: if you do not get any data back, run the report SAPBC_DATA_GENERATOR
	//       through SE38 in your SAP system
	HRESULT hr = session->CallFunction(fn, VARIANT_TRUE);
	if (hr != S_OK || session->Error)
	{
		cout << "ERROR:" << session->ErrorInfo->Message << "\n";
		leave(-1);
	}

	// process the result
	// NOTE: all indexes in RfcConnector are 1-based
	for (long i = 1; i <= fn->Tables->Item[L"FLIGHT_LIST"]->Rows->Count; i++)
	{
		IRfcFieldsPtr row = fn->Tables->Item[L"FLIGHT_LIST"]->Rows->Item[i];
		cout << (_bstr_t)row->Item[L"AIRLINE"]->value
			<< "\t"
			<< (_bstr_t)row->Item[L"FLIGHTDATE"]->value
			<< "\t"
			<< (_bstr_t)row->Item[L"CITYFROM"]->value
			<< "\t"
			<< (_bstr_t)row->Item[L"CITYTO"]->value
			<< "\n";
	}
}

void read_table(IRfcSessionPtr &session) {
	ITableReaderPtr tr = session->GetTableReader(L"SFLIGHT");

	// set up query parameters: only return rows where CARRID equals 'LH'
	tr->Query->Add(L"CARRID EQ 'LH'");

	// read the table rows, starting from row 0, with no limit
	tr->Read(0, 0);

	// process the result
	for (long i = 1; i <= tr->Rows->Count; i++)
	{
		IRfcFieldsPtr row = tr->Rows->Item[i];
		cout << (_bstr_t)row->Item[L"CARRID"]->value
  			 << "\t"
			 << (_bstr_t)row->Item[L"CONNID"]->value
			 << "\t"
			 << (_bstr_t)row->Item[L"FLDATE"]->value
			 << "\t"
			 << (_bstr_t)row->Item[L"PRICE"]->value
			 << "\n";
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	// init COM & create RfcSession instance
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	IRfcSessionPtr rfc;	
	HRESULT hr = rfc.CreateInstance(L"RfcConnector.NwRfcSession");
	if (hr != S_OK)
	{
		cout << "Error: could not create RfcSession instance, hr=0x" << hex << hr << "\n";
		leave(-1);		
	}

	// initialize system, user, password
	logon(rfc);

	// call a function / BAPI
	call_function(rfc);
	
	// read a table
	read_table(rfc);

	leave(0);
}

