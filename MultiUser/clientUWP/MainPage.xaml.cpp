//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "SocketStuff.h"
#include <thread>
#include <atomic>

using namespace ChitChatTCP;

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

//Global Variables
ClientTCP client;
bool connected = false;
std::atomic_bool state;
std::atomic_bool terminated;
std::chrono::time_point<std::chrono::steady_clock> prevRecv;

MainPage::MainPage()
{
	state.store(false, std::memory_order_relaxed);
	terminated.store(true, std::memory_order_relaxed);
	InitializeComponent();
}

MainPage::~MainPage()
{
	//ensure any daemon thread will get closed
	state.store(false, std::memory_order_relaxed);
	while (!terminated.load(std::memory_order_relaxed))
	{
		//spin until the daemon thread closes
	}
}

//From
//	https://stackoverflow.com/questions/32991246/c-cx-convert-stdstring-to-platformstring
Platform::String^ convertFromString(const std::string& input)
{
	std::wstring w_str = std::wstring(input.begin(), input.end());
	const wchar_t* w_chars = w_str.c_str();

	return (ref new Platform::String(w_chars, w_str.length()));
}

//From
//	https://stackoverflow.com/questions/28759212/convert-platformstring-to-stdstring
std::string convertToString(Platform::String^ _in)
{
	std::wstring temp(_in->Begin());
	return std::string(temp.begin(), temp.end());
}

void ChitChatTCP::MainPage::send_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	std::string message = convertToString(textBox->Text);
	if (message.size() > 0)
	{
		//Only send a message if non-zero, lest server think client leaving
		//Reset the text entry to nothing for next message
		textBox->Text = "";
		std::string result = client.sendConnection(message);
		if (result.size() == 0)
		{
			state.store(false, std::memory_order_relaxed);
			listBox1->Items->Append("LOCAL: Connection either closed by server, or error occurred.");
			listBox1->SelectedIndex = listBox1->Items->Size - 1;
			myButton->Content = "Connect";
		}
	}
}

/*void MainPage::updateListBox(Platform::String^ val)
{
	listBox1->Items->Append(val);
	listBox1->SelectedIndex = listBox1->Items->Size - 1;
}
void MainPage::updateButton(Platform::String^ val)
{
	
}

void TestFunction(Windows::UI::Xaml::Controls::ListBox^ listBox1,
	Windows::UI::Xaml::Controls::Button^ myButton)
{
	//Reset the prevRecv time clock - use this to terminate if inactive for 5 minutes
	prevRecv = std::chrono::steady_clock::now();
	//ChitChatTCP::MainPage::updateListBox(convertFromString("testing"));
	while (state.load(std::memory_order_relaxed))
	{
		std::string result = client.receiveConnection(prevRecv);
		if (result.size() > 0)
		{
			if (result[0] != '\b')
			{
				//listBox1->Items->Append(convertFromString(result));
				//listBox1->SelectedIndex = listBox1->Items->Size - 1;
			}
		}
		else
		{
			//terminate this thread, likely a problem occurred somewhere or server closed
			state.store(false, std::memory_order_relaxed);
			//listBox1->Items->Append("LOCAL: Connection either closed by server, or error occurred.");
			//listBox1->SelectedIndex = listBox1->Items->Size - 1;
			//myButton->Content = "Connect";
		}
		//Condition to close the connection if inactive for 5+ minutes
		if (std::chrono::duration_cast<std::chrono::minutes>(std::chrono::steady_clock::now() - prevRecv).count() >= 5)
		{
			//listBox1->Items->Append("LOCAL: Inactive for 5 minutes, closing connection.");
			//listBox1->SelectedIndex = listBox1->Items->Size - 1;
			client.closeConnection();
			state.store(false, std::memory_order_relaxed);
		}
	}
	//listBox1->Items->Append("LOCAL: Receive thread terminating...");
	//listBox1->SelectedIndex = listBox1->Items->Size - 1;
	terminated.store(true, std::memory_order_relaxed);
}*/

void ChitChatTCP::MainPage::myButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (!connected)
	{
		connected = true;
		std::string result = client.openConnection();
		if (result.size() == 0)myButton->Content = "Disconnect";
		else
		{
			listBox1->Items->Append("CONNECT ERROR: " + convertFromString(result));
			listBox1->SelectedIndex = listBox1->Items->Size - 1;
			return;
		}
		//no issues, start the receive thread
		state.store(true, std::memory_order_relaxed);
		terminated.store(false, std::memory_order_relaxed);
		std::thread recvThread([&] {receiveLoop(); });//&TestFunction, listBox1, myButton);
		recvThread.detach();//Daemon thread
	}
	else
	{
		connected = false;
		state.store(false, std::memory_order_relaxed);
		while (!terminated.load(std::memory_order_relaxed))
		{
			//spin until the daemon thread closes
		}
		client.closeConnection();
		//Get the Daemon thread to terminate...
		state.store(false, std::memory_order_relaxed);
		myButton->Content = "Connect";
	}
}



void ChitChatTCP::MainPage::receiveLoop()
{
	//Reset the prevRecv time clock - use this to terminate if inactive for 5 minutes
	prevRecv = std::chrono::steady_clock::now();
	while (state.load(std::memory_order_relaxed))
	{
		std::string result = client.receiveConnection(prevRecv);
		if (result.size() > 0)
		{
			if (result[0] != '\b')
			{
				Platform::String^ temp = convertFromString(result);
				listBox1->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
					ref new Windows::UI::Core::DispatchedHandler([this, temp]
						{
							listBox1->Items->Append(temp);
							listBox1->SelectedIndex = listBox1->Items->Size - 1;
						}));
			}
		}
		else
		{
			//terminate this thread, likely a problem occurred somewhere or server closed
			/*state.store(false, std::memory_order_relaxed);
			listBox1->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
				ref new Windows::UI::Core::DispatchedHandler([this]
					{
						listBox1->Items->Append("LOCAL: Connection either closed by server, or error occurred.");
						listBox1->SelectedIndex = listBox1->Items->Size - 1;
					}));*/
			//myButton->Content = "Connect";
		}
		//Condition to close the connection if inactive for 5+ minutes
		if (std::chrono::duration_cast<std::chrono::minutes>(std::chrono::steady_clock::now() - prevRecv).count() >= 5)
		{
			listBox1->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
				ref new Windows::UI::Core::DispatchedHandler([this]
					{
						listBox1->Items->Append("LOCAL: Inactive for 5 minutes, closing connection.");
						listBox1->SelectedIndex = listBox1->Items->Size - 1;
					}));
			client.closeConnection();
			state.store(false, std::memory_order_relaxed);
		}
	}
	listBox1->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
		ref new Windows::UI::Core::DispatchedHandler([this]
			{
				listBox1->Items->Append("LOCAL: Receive thread terminating...");
				listBox1->SelectedIndex = listBox1->Items->Size - 1;
			}));
	terminated.store(true, std::memory_order_relaxed);
}
