//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include <string>

namespace ChitChatTCP
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();
		virtual ~MainPage();

		//void updateListBox(Platform::String^ val);
		//void updateButton(Platform::String^ val);

	private:
		void send_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void myButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void receiveLoop();
	};
}
