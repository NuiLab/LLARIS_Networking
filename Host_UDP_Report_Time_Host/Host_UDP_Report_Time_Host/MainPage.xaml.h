//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

namespace Host_UDP_Report_Time_Host
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

	private:
		void SendNew_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void SendZero_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void GetClient_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
