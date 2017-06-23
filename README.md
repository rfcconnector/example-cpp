# RFC Connector: (unmanaged) C++ Client Example

This example demonstrates how to connect SAP from unmanaged C++ using RfcConnector. 
It shows how to call Function Modules (BAPIs) and how to read table data from an SAP 
server/backend.

## Importing the type library

Use the [`#import` directive](https://msdn.microsoft.com/en-us/library/8etzzkb6.aspx) to import the Rfc Connector TLB, 
and let the compiler automatically generate COM wrappers:

```cpp
#import "libid:FBDBDBA5-518B-4146-A52F-455C0E03492D" no_namespace
```

## Connecting to the SAP system

Rfc Connector supports three session classes:

* `NWRfcSession` (recommended with SAP GUI 7.50+)
* `RfcSession` (legacy, use only if you cannot upgrade)
* `SoapSession` (http(s) connection, use if you don't want to install additional libraries. Does not support SSO and Visual RFC.)

To create a session, just create the COM instance:

```cpp
// initialize COM
CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

IRfcSessionPtr session;	
RESULT hr = session.CreateInstance(L"RfcConnector.NwRfcSession");
if (hr != S_OK)
{
	// handle error
}
```

Then, set destination and logon credentials and call `Connect()`:

```cpp
session->RfcSystemData->ConnectString = L"SAPLOGON_ID=NPL";
session->LogonData->Client = L"001";
session->LogonData->User = L"DEVELOPER";
session->LogonData->Password = L"developer1";
session->LogonData->Language = L"EN";

hr = session->Connect();
if (hr != S_OK || session->Error) 
{
	// handle error
}
```

## Calling a function module/BAPI

To call a function module or BAPI, import the function prototype, set the parameters, 
then call it and process the result:

```cpp
// import function
IFunctionCallPtr fn = session->ImportCall(L"BAPI_FLIGHT_GETLIST",VARIANT_TRUE);

// set parameters
fn->Importing->Item[L"AIRLINE"]->value = L"LH";

// call the function
hr = session->CallFunction(fn, VARIANT_TRUE);
if (hr != S_OK || rfc->Error)
{
	// handle error
}

// read the result
for (long i=1;i<=fn->Tables->Item[L"FLIGHT_LIST"]->Rows->Count;i++) 
{
	IRfcFieldsPtr row = fn->Tables->Item[L"FLIGHT_LIST"]->Rows->Item[i];
	cout << (_bstr_t)row->Item[L"AIRLINE"]->value; // ...
}
```

## Reading database tables

To read data from a database table, just create an instance of `TableReader`, call `read()`
and process the result:

```cpp
ITableReaderPtr tr = session->GetTableReader(L"SFLIGHT");

// set up query parameters: only return rows where CARRID equals 'LH'
tr->Query->Add(L"CARRID EQ 'LH'");

// read the table rows, starting from row 0, with no limit
tr->Read(0, 0);

// process the result
for (long i = 1; i <= tr->Rows->Count; i++)
{
	IRfcFieldsPtr row = tr->Rows->Item[i];
	cout << (_bstr_t)row->Item[L"CARRID"]->value; // ...
}
```

Note: For limitations of this approach and possible workarounds, 
please refer to the [knowledgebase entry](http://rfcconnector.com/documentation/kb/0007/)