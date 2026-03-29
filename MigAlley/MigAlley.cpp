// MigAlley.cpp : Defines the entry point for the console application.
//
#include "WIN32_COMPAT.H"
#include "MFC_stub.h"
#include "DLGITEM.H"
#include "RDIALOG.H"
#include "MIG.H"
#include "MIGVIEW.H"


/////////////////////////////////////////////////////////////////
// The one and only CMIGApp object
CMIGApp theApp;

int main(int argc, char** argv)
{
    if (!theApp.InitInstance())
        return -1;

    return theApp.Run();
}

/////////////////////////////////////////////////////////////////
