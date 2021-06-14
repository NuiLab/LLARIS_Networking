//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//


#include "pch.h"
#include "MainPage.xaml.h"
#include "SocketStuff.h"



using namespace HelloWorld_UWP_CXX;

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

MainPage::MainPage()
{
	InitializeComponent();
}

//From
//	https://stackoverflow.com/questions/32991246/c-cx-convert-stdstring-to-platformstring
Platform::String^ convertFromString(const std::string& input)
{
	std::wstring w_str = std::wstring(input.begin(), input.end());
	const wchar_t* w_chars = w_str.c_str();

	return (ref new Platform::String(w_chars, w_str.length()));
}

void HelloWorld_UWP_CXX::MainPage::Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	myButton->Content = "Hrrm";
	Communicate->Text = convertFromString(attempt());
}
