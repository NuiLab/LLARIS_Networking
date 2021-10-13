//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "UDPSendTerminate.h"

using namespace Host_UDP_Report_Time_Host;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

bool connected = false;

Platform::String^ convertFromString(const std::string& input)
{
	std::wstring w_str = std::wstring(input.begin(), input.end());
	const wchar_t* w_chars = w_str.c_str();

	return (ref new Platform::String(w_chars, w_str.length()));
}

std::string convertToString(Platform::String^ _in)
{
	std::wstring temp(_in->Begin());
	return std::string(temp.begin(), temp.end());
}

MainPage::MainPage()
{
	InitializeComponent();
}


void Host_UDP_Report_Time_Host::MainPage::SendNew_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	std::string info;
	if (connected)
	{
		info = sendInteger();
	}
	else
	{
		info = connectUDP();
		connected = true;
	}
	Header->Text = convertFromString(info);
}


void Host_UDP_Report_Time_Host::MainPage::SendZero_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (connected)sendZero();
	Header->Text = "Terminated";
}


void Host_UDP_Report_Time_Host::MainPage::GetClient_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Header->Text = convertFromString(gatherClient());
}
