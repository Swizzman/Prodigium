#include "InputHandler.h"
InputHandler* InputHandler::instance = nullptr;
InputHandler::InputHandler()
{
	lol = 0;
}
InputHandler::~InputHandler()
{
	if (InputHandler::instance)
	{
		delete InputHandler::instance;
	}
}

bool InputHandler::Initialize(const HWND& windowHandle)
{
	if (!InputHandler::instance)
	{
		InputHandler::instance = new InputHandler;
		InputHandler::instance->keyboard = std::make_unique<DirectX::Keyboard>();
		InputHandler::instance->mouse = std::make_unique<DirectX::Mouse>();
		InputHandler::instance->mouse->SetWindow(windowHandle);
	}
	else
	{
		std::cerr << "InputHandler already Initialized\n";
}
	return true;

}

Keyboard::State InputHandler::getKBState()
{
	return InputHandler::instance->keyboard->GetState();
}

Mouse::State InputHandler::getMouseState()
{
	return InputHandler::instance->mouse->GetState();
}

LRESULT InputHandler::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Sort through and find what code to run for the message given
	switch (message)
	{
	case WM_ACTIVATEAPP:
		Keyboard::ProcessMessage(message, wParam, lParam);
		Mouse::ProcessMessage(message, wParam, lParam);
		break;
		// This message is read when the window is closed
	case WM_DESTROY:
		// Tell calling thread to terminate
		PostQuitMessage(0);
		return 0;
	case WM_KEYDOWN:
		break;
	case WM_MOUSEMOVE:
		break;
	case WM_LBUTTONDOWN:
		break;
	case WM_RBUTTONDOWN:
		break;
	case WM_MOUSEHOVER:
		Mouse::ProcessMessage(message, wParam, lParam);
		break;
	case WM_SYSKEYDOWN:
		Keyboard::ProcessMessage(message, wParam, lParam);
		break;

	}

	// Handle any messages the switch statement didn't by using default methodology
	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT InputHandler::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Sort through and find what code to run for the message given
	switch (message)
	{
		// This message is read when the window is closed
	case WM_DESTROY:
		// Tell calling thread to terminate
		PostQuitMessage(0);
		return 0;
	case WM_KEYDOWN:
		break;
	case WM_MOUSEMOVE:
		break;
	case WM_LBUTTONDOWN:
		break;
	case WM_RBUTTONDOWN:
		break;
	}

	// Handle any messages the switch statement didn't by using default methodology
	return DefWindowProc(hWnd, message, wParam, lParam);
}