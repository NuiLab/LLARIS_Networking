//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "SocketData.h"

using namespace UWPUDPPerformanceCXX;

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

void UWPUDPPerformanceCXX::MainPage::Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	/*
	Platform::String^ appInstallFolder = Windows::ApplicationModel::Package::Current->InstalledLocation->Path; //where program in installed. Have read only access
	Platform::String^ localfolder = Windows::Storage::ApplicationData::Current->LocalFolder->Path;	//for local saving for future
	Platform::String^ roamingFolder = Windows::Storage::ApplicationData::Current->RoamingFolder->Path;	//for sync between devices
	Platform::String^ temporaryFolder = Windows::Storage::ApplicationData::Current->TemporaryFolder->Path;	//for temp saving. Cleared often by system
	*/
	Platform::String^ localfolder = Windows::Storage::ApplicationData::Current->LocalFolder->Path;
	std::wstring widePath(localfolder->Begin());
	std::string path(widePath.begin(), widePath.end());
	myButton->Content = "Hrrm";
	Communicate->Text = "Fin";
	listBox1->Items->Append(convertFromString(attempt(path)));
	listBox1->SelectedIndex = listBox1->Items->Size - 1;
}