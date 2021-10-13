//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "UDPSocketLoop.h"

using namespace HL2_UDP_Report_Time_Client;

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
using namespace Windows::Storage;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

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


void HL2_UDP_Report_Time_Client::MainPage::myButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Header->Text = convertFromString(attempt(convertToString(ApplicationData::Current->LocalFolder->Path))); //"Finished Collecting Times";
}
