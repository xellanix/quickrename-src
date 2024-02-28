// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
#if __has_include("ExplorerItem.g.cpp")
#include "ExplorerItem.g.cpp"
#endif

#pragma region WinUI Headers

#include "microsoft.ui.xaml.window.h"
#include "winrt/Microsoft.UI.Interop.h"
#include "winrt/Microsoft.UI.Windowing.h"
#include "winrt/Microsoft.UI.Composition.h"
#include "winrt/Microsoft.UI.Xaml.Hosting.h"
#include "winrt/Microsoft.UI.Xaml.Input.h"
#include "winrt/Microsoft.UI.Text.h"

#pragma endregion

#pragma region Other Headers

#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")
#include <shellscalingapi.h>
#pragma comment(lib, "Shcore.lib")

#pragma endregion

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Windowing;

#define IS_ON_DEVELOPMENT_PROCESS 0

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::QuickRename::implementation
{
    MainWindow::MainWindow()
    {
        #if IS_ON_DEVELOPMENT_PROCESS

        ShellExecute(NULL, NULL, L"D:\\Beta Projects\\AppSystemUsage\\x64\\Debug\\AppSystemUsage.exe",
                     (std::to_wstring(GetCurrentProcessId())).c_str(),
                     NULL, SW_SHOWMINIMIZED);

        #endif // IS_ON_DEVELOPMENT_PROCESS

        InitializeComponent();

        SetModernAppTitleBar({ 1088, 680 });
    }

    #pragma region Modern Title Section

    double GetScaleFactorFromHMonitor(HMONITOR const& hMonitor)
    {
        // Get DPI.
        UINT dpiX, dpiY;
        int result = GetDpiForMonitor(hMonitor, MDT_DEFAULT, &dpiX, &dpiY);
        if (result != S_OK)
        {
            return 1.0;
        }

        auto scaleFactorPercent = dpiX / 96.0;
        return scaleFactorPercent;
    }

    void MainWindow::SetModernAppTitleBar(Windows::Graphics::SizeInt32 windowSize)
    {
        auto windowNative{ this->try_as<::IWindowNative>() };
        winrt::check_bool(windowNative);
        windowNative->get_WindowHandle(&Xellanix::Desktop::WindowHandle);

        Microsoft::UI::WindowId windowId =
            Microsoft::UI::GetWindowIdFromWindow(Xellanix::Desktop::WindowHandle);

        Microsoft::UI::Windowing::AppWindow appWindow =
            Microsoft::UI::Windowing::AppWindow::GetFromWindowId(windowId);

        if (auto presenter = appWindow.Presenter().try_as<OverlappedPresenter>())
        {
            HMONITOR hMonitor;
            {
                POINT _cursor;
                if (GetCursorPos(&_cursor) == FALSE) _cursor = { 0, 0 };
                hMonitor = MonitorFromPoint(_cursor, MONITOR_DEFAULTTONEAREST);
            }

            // The window size if the DPI Scale Factor is 1.25 (125%)
            {
                auto scaleAdj = GetScaleFactorFromHMonitor(hMonitor);

                auto newWidth = static_cast<int32_t>(windowSize.Width / 1.25 * scaleAdj);
                auto newHeight = static_cast<int32_t>(windowSize.Height / 1.25 * scaleAdj);

                windowSize = Windows::Graphics::SizeInt32{ newWidth, newHeight };
            }
            appWindow.Resize(windowSize);

            MONITORINFO info{};
            info.cbSize = sizeof(decltype(info));

            if (GetMonitorInfo(hMonitor, &info) != 0)
            {
                const auto width = windowSize.Width;
                const auto height = windowSize.Height;
                auto px = (info.rcWork.right - width) / 2;
                auto py = (info.rcWork.bottom - height) / 2;

                appWindow.Move(Windows::Graphics::PointInt32{ px, py });
            }
        }

        if (AppWindowTitleBar::IsCustomizationSupported())
        {
            auto titleBar{ appWindow.TitleBar() };

            titleBar.ExtendsContentIntoTitleBar(true);
            titleBar.ButtonBackgroundColor(Microsoft::UI::Colors::Transparent());
            titleBar.ButtonInactiveBackgroundColor(Microsoft::UI::Colors::Transparent());
        }
        else
        {
            // Title bar customization using these APIs is currently
            // supported only on Windows 11. In other cases, hide
            // the custom title bar element.            
            AppTitleBar().Visibility(Visibility::Collapsed);

            // Show alternative UI for any functionality in
            // the title bar, such as search.
        }

        if (appWindow)
        {
            // You now have an AppWindow object, and you can call its methods to manipulate the window.
            // As an example, let's change the title text of the window.
            appWindow.Title(L"Xellanix PickColor");
            SetTitleBarTheme();
        }
    }

    double MainWindow::GetScaleAdjustment()
    {
        Microsoft::UI::WindowId windowId =
            Microsoft::UI::GetWindowIdFromWindow(Xellanix::Desktop::WindowHandle);

        DisplayArea displayArea = DisplayArea::GetFromWindowId(windowId, DisplayAreaFallback::Nearest);
        auto hMonitor = Microsoft::UI::GetMonitorFromDisplayId(displayArea.DisplayId());

        return GetScaleFactorFromHMonitor(hMonitor);
    }

    void MainWindow::SetDragArea()
    {
        auto scaleAdj = GetScaleAdjustment();

        Microsoft::UI::WindowId windowId =
            Microsoft::UI::GetWindowIdFromWindow(Xellanix::Desktop::WindowHandle);

        Microsoft::UI::Windowing::AppWindow appWindow =
            Microsoft::UI::Windowing::AppWindow::GetFromWindowId(windowId);

        if (AppWindowTitleBar::IsCustomizationSupported())
        {
            auto titleBar{ appWindow.TitleBar() };

            const auto buttonWidth = GetSystemMetrics(SM_CXSIZE);
            RightPaddingColumn().Width(mux::GridLengthHelper::FromPixels(buttonWidth));

            const auto dragWidth = static_cast<int32_t>(LeftDragColumn().ActualWidth() * scaleAdj);
            const auto dragHeight = static_cast<int32_t>(32 * scaleAdj);
            titleBar.SetDragRectangles({ Windows::Graphics::RectInt32{ 0, 0, dragWidth, dragHeight } });
        }
        else
        {
            // Title bar customization using these APIs is currently
            // supported only on Windows 11. In other cases, hide
            // the custom title bar element.            
            AppTitleBar().Visibility(Visibility::Collapsed);

            // Show alternative UI for any functionality in
            // the title bar, such as search.
        }

        HMONITOR hMonitor;
        {
            POINT _cursor;
            if (GetCursorPos(&_cursor) == FALSE) _cursor = { 0, 0 };
            hMonitor = MonitorFromPoint(_cursor, MONITOR_DEFAULTTONEAREST);
        }

        MONITORINFO info{};
        info.cbSize = sizeof(decltype(info));

        if (GetMonitorInfo(hMonitor, &info) != 0)
        {
            const auto windowSize = appWindow.Size();

            const auto width = windowSize.Width;
            const auto height = windowSize.Height;
            auto px = (info.rcWork.right - width) / 2;
            auto py = (info.rcWork.bottom - height) / 2;

            appWindow.Move(Windows::Graphics::PointInt32{ px, py });
        }
    }

    bool MainWindow::GetCurrentTheme()
    {
        bool isCurrentDarkMode = false;
        if (auto theme{ m_rootElement.RequestedTheme() }; theme == ElementTheme::Dark)
        {
            isCurrentDarkMode = true;
        }
        else if (theme == ElementTheme::Light)
        {
            isCurrentDarkMode = false;
        }
        else
        {
            if (auto currentApp{ Application::Current() })
            {
                isCurrentDarkMode = currentApp.RequestedTheme() == ApplicationTheme::Dark;
            }
        }

        return isCurrentDarkMode;
    }

    void MainWindow::SetTitleBarTheme()
    {
        // Application theme.
        m_rootElement = this->Content().try_as<winrt::mux::FrameworkElement>();
        if (nullptr != m_rootElement)
        {
            m_themeChangedRevoker = m_rootElement.ActualThemeChanged(winrt::auto_revoke, [&](auto&&, auto&&)
            {
                bool isCurrentDarkMode = GetCurrentTheme();
                if (isDarkMode == isCurrentDarkMode) return;
                isDarkMode = isCurrentDarkMode;

                BOOL value = isDarkMode;

                ::DwmSetWindowAttribute(Xellanix::Desktop::WindowHandle, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));

                if (AppWindowTitleBar::IsCustomizationSupported())
                {
                    auto foregroundColor{ Microsoft::UI::ColorHelper::FromArgb(255, 48, 48, 48) };
                    auto hoverBackground{ Microsoft::UI::ColorHelper::FromArgb(255, 230, 230, 230) };
                    auto preseedBackground{ Microsoft::UI::ColorHelper::FromArgb(255, 242, 242, 242) };
                    if (isDarkMode)
                    {
                        foregroundColor = Microsoft::UI::ColorHelper::FromArgb(255, 245, 245, 247);
                        hoverBackground = Microsoft::UI::ColorHelper::FromArgb(255, 51, 51, 51);
                        preseedBackground = Microsoft::UI::ColorHelper::FromArgb(255, 38, 38, 38);
                    }

                    Microsoft::UI::WindowId windowId =
                        Microsoft::UI::GetWindowIdFromWindow(Xellanix::Desktop::WindowHandle);

                    Microsoft::UI::Windowing::AppWindow appWindow =
                        Microsoft::UI::Windowing::AppWindow::GetFromWindowId(windowId);

                    auto titleBar{ appWindow.TitleBar() };

                    titleBar.ButtonForegroundColor(foregroundColor);
                    titleBar.ButtonHoverForegroundColor(foregroundColor);
                    titleBar.ButtonHoverBackgroundColor(hoverBackground);
                    titleBar.ButtonPressedForegroundColor(foregroundColor);
                    titleBar.ButtonPressedBackgroundColor(preseedBackground);
                    titleBar.ButtonInactiveForegroundColor(foregroundColor);
                }
            });
        }
    }

    #pragma endregion

    #pragma region Page & Startup Events

    Windows::Foundation::IAsyncAction MainWindow::ProcessActivationInfo()
    {
        using namespace Microsoft::Windows::AppLifecycle;

        apartment_context ui_thread;
        auto weak{ get_weak() };
        co_await resume_background();

        AppActivationArguments args = AppInstance::GetCurrent().GetActivatedEventArgs();
        if (args.Kind() == ExtendedActivationKind::Launch)
        {
            if (auto launchArgs{ args.Data().try_as<Windows::ApplicationModel::Activation::ILaunchActivatedEventArgs>() })
            {
                std::wstring argString{ launchArgs.Arguments() };

                auto pos = argString.find(L"--");
                if (pos == std::wstring::npos) co_return;
                {
                    auto space = argString.find(L" ", pos + 2);

                    argString = argString.substr(space + 1);
                }

                std::vector<hstring> files;
                files.reserve(1'000'000);

                std::wistringstream iss(argString);
                for (std::wstring s; iss >> std::quoted(s, L'"', L'\000'); )
                {
                    files.emplace_back(s);
                }
                files.shrink_to_fit();

                if (auto strong{ weak.get() })
                {
                    strong->SortItems(files);

                    co_await ui_thread;

                    strong->m_explorerItems.Clear();
                    swapreset(strong->m_explorerItemsSet);
                    strong->m_recursiveSetted = false;

                    for (auto const& s : files)
                    {
                        co_await strong->AddExplorerItem(s);
                    }
                }
            }
        }
    }

    void MainWindow::WindowContentLoaded(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        using namespace std::chrono_literals;

        LoadSettings();

        constexpr bool morethan0 = false;
        DeleteItemBtn().IsEnabled(morethan0);
        ApplyBtn().IsEnabled(morethan0);
        ClearItemBtn().IsEnabled(morethan0);
        SelectAllBtn().IsEnabled(morethan0);
        InvertBtn().IsEnabled(morethan0);
        NoFilePanel().Visibility(Visibility::Visible);

        if (!vectorToken)
        {
            vectorToken = m_explorerItems.VectorChanged(auto_revoke, [&](auto&&, auto&&)
            {
                bool _morethan0 = m_explorerItems.Size() > 0;
                ClearItemBtn().IsEnabled(_morethan0);
                SelectAllBtn().IsEnabled(_morethan0);
                InvertBtn().IsEnabled(_morethan0);

                if (_morethan0)
                {
                    NoFilePanel().Visibility(Visibility::Collapsed);
                }
                else
                {
                    NoFilePanel().Visibility(Visibility::Visible);

                    swapreset(m_explorerItemsSet);
                    swapreset(m_selectedIndex);
                    swapreset(m_rememberItems);
                }
            });
        }

        no_await(ProcessActivationInfo());

        fileChecker.Interval(500ms);
        if (!fileChecker_revoker)
        {
            fileChecker_revoker = fileChecker.Tick(auto_revoke, [&](auto&&, auto&&)
            {
                struct _stati64 current_buf1;
                auto res1 = _wstat64((Xellanix::Utilities::LocalAppData / L"Settings\\QuickRename.xsmf").wstring().c_str(), &current_buf1);

                if (res1 == 0)
                {
                    if (current_buf1.st_ctime != last_ctime1 ||
                        current_buf1.st_mtime != last_mtime1 ||
                        current_buf1.st_size != last_size1)
                    {
                        LoadSettings();

                        last_ctime1 = current_buf1.st_ctime;
                        last_mtime1 = current_buf1.st_mtime;
                        last_size1 = current_buf1.st_size;
                    }
                }
            });
        }
        fileChecker.Start();
    }

    #pragma endregion
    
    #pragma region File List
    
    Windows::Foundation::IAsyncAction MainWindow::FileListChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& /*e*/)
    {
        auto items{ FileList().SelectedRanges() };

        // Enable or Disable Delete and Merge Button basedon 'items' size
        bool morethan0 = items.Size() > 0 ? true : false;
        DeleteItemBtn().IsEnabled(morethan0);
        ApplyBtn().IsEnabled(morethan0);

        /*if (setNewNameOp)
        {
            if (setNewNameOp.Status() != Windows::Foundation::AsyncStatus::Completed)
            {
                setNewNameOp.Cancel();
            }
        }*/
        setNewNameOps.Clear();
        swapreset(m_selectedIndex);
        for (auto const&& i : items)
        {
            uint32_t start{ static_cast<uint32_t>(i.FirstIndex()) };
            for (uint32_t index = start; index < start + i.Length(); index++)
            {
                m_selectedIndex.insert(index);
            }
        }

        auto canReorder = m_selectedIndex.size() < 2;
        FileList().CanReorderItems(canReorder);
        FileList().CanDragItems(canReorder);
        
        setNewNameOps.Replace(SetNewName());
        co_return;
        /*setNewNameOp = SetNewName();
        co_await setNewNameOp;*/
    }

    void MainWindow::FilesDragOver(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Microsoft::UI::Xaml::DragEventArgs const& e)
    {
        e.AcceptedOperation(Windows::ApplicationModel::DataTransfer::DataPackageOperation::Copy);
    }
    
    Windows::Foundation::IAsyncAction MainWindow::FilesDrop(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Microsoft::UI::Xaml::DragEventArgs const& e)
    {
        auto strong_this{ get_strong() };

        if (e.DataView().Contains(Windows::ApplicationModel::DataTransfer::StandardDataFormats::StorageItems()))
        {
            auto&& items{ co_await e.DataView().GetStorageItemsAsync() };
            if (items.Size() == 0)
                co_return;

            strong_this->m_recursiveSetted = false;

            std::vector<hstring> temp;
            temp.reserve(items.Size());
            for (auto&& item : items)
            {
                if (item.IsOfType(Windows::Storage::StorageItemTypes::File))
                {
                    if (auto file{ item.try_as<Windows::Storage::StorageFile>() })
                    {
                        temp.emplace_back(file.Path());
                    }
                }
                else if (item.IsOfType(Windows::Storage::StorageItemTypes::Folder))
                {
                    if (auto folder{ item.try_as<Windows::Storage::StorageFolder>() })
                    {
                        temp.emplace_back(folder.Path());
                    }
                }
            }
            temp.shrink_to_fit();

            strong_this->SortItems(temp);
            for (auto const& item : temp)
            {
                co_await strong_this->AddExplorerItem(item);
            }
        }
    }

    #pragma endregion

    #pragma region ControlPanel Buttons Action

    Windows::Foundation::IAsyncAction MainWindow::AddClick(winrt::Microsoft::UI::Xaml::Controls::SplitButton const& /*sender*/, winrt::Microsoft::UI::Xaml::Controls::SplitButtonClickEventArgs const& /*e*/)
    {
        co_await ShowFilesPicker();
    }

    Windows::Foundation::IAsyncAction MainWindow::AddFilesFolderClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Microsoft::UI::Xaml::Controls::ItemClickEventArgs const& e)
    {
        auto type = e.ClickedItem().as<hstring>();
        if (type == L"Files")
        {
            co_await ShowFilesPicker();
        }
        else if (type == L"Folders")
        {
            auto strong_this{ get_strong() };

            Windows::Storage::Pickers::FolderPicker picker{};
            picker.as<Xellanix::Desktop::IInitializeWithWindow>()->Initialize(Xellanix::Desktop::WindowHandle);
            picker.FileTypeFilter().Append(L"*");
            std::vector<hstring> folders;

            while (true)
            {
                auto&& folder{ co_await picker.PickSingleFolderAsync() };
                if (!folder)
                {
                    break;
                }
                else
                {
                    folders.emplace_back(folder.Path());
                }
            }

            if (folders.empty()) co_return;

            strong_this->m_recursiveSetted = false;
            strong_this->SortItems(folders);
            for (auto& folder : folders)
            {
                co_await strong_this->AddExplorerItem(folder);
            }
        }
    }

    void MainWindow::DeleteItemClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto list{ FileList() };
        if (!list) return;

        std::vector<uint32_t> removed;
        removed.reserve(list.Items().Size());

        for (auto&& i : list.SelectedRanges())
        {
            uint32_t start{ static_cast<uint32_t>(i.FirstIndex()) };
            for (uint32_t index = start; index < start + i.Length(); index++)
            {
                removed.push_back(index);
            }
        }

        removed.shrink_to_fit();
        std::reverse(removed.begin(), removed.end());
        list.DeselectRange(Data::ItemIndexRange(0, list.Items().Size()));

        for (auto& i : removed)
        {
            m_explorerItemsSet.erase(m_explorerItems.GetAt(i).try_as<QuickRename::ExplorerItem>().ItemPath().c_str());
            m_explorerItems.RemoveAt(i);
        }
    }

    void MainWindow::ClearItemClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        m_explorerItems.Clear();

        swapreset(m_explorerItemsSet);
    }

    void MainWindow::SelectAllClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (auto list{ FileList() })
            list.SelectAll();
    }

    void MainWindow::InvertSelectClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto list{ FileList() };
        if (!list) return;

        using range_t = Data::ItemIndexRange;
        std::vector<range_t> invertedRange;
        {
            const auto available = list.Items().Size();
            std::vector<range_t> selectedRange;
            selectedRange.reserve(list.SelectedRanges().Size());
            for (auto&& i : list.SelectedRanges())
            {
                selectedRange.push_back(i);
            }

            if (selectedRange.size() > 0)
            {
                bool is_selectedFirst = selectedRange.front().FirstIndex() != 0;
                bool is_selectedLast = selectedRange.back().FirstIndex() + selectedRange.back().Length() != available;
                short to_reserve = (short)is_selectedFirst + (short)is_selectedLast - 1;
                invertedRange.reserve(selectedRange.size() + to_reserve);

                typedef decltype(range_t{ 0, 1 }.FirstIndex()) first_t;
                typedef decltype(range_t{ 0, 1 }.Length()) length_t;

                for (size_t i = 0; i < selectedRange.size(); i++)
                {
                    auto&& val_t = selectedRange[i];
                    if (val_t.FirstIndex() == 0) continue;
                    auto&& before_t = i == 0 ? range_t{ 0, 0 } : selectedRange[i - 1];

                    auto before = static_cast<first_t>(before_t.FirstIndex() + before_t.Length());
                    auto length = static_cast<length_t>((std::max)(val_t.FirstIndex() - before, 1));
                    range_t range{ before, length };
                    invertedRange.push_back(std::move(range));
                }

                if (auto lastval = static_cast<first_t>(selectedRange.back().FirstIndex() + selectedRange.back().Length()); static_cast<length_t>(lastval) != available)
                {
                    range_t range{ lastval, (std::max)(available - lastval, (length_t)1) };
                    invertedRange.push_back(std::move(range));
                }
            }
            else
            {
                invertedRange.reserve(1);
                range_t range{ 0, available };
                invertedRange.push_back(std::move(range));
            }
        }

        list.DeselectRange(range_t(0, list.Items().Size()));
        for (auto&& range : invertedRange)
        {
            list.SelectRange(range);
        }
    }

    void MainWindow::DeselectClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (auto list{ FileList() })
            list.DeselectRange(Data::ItemIndexRange(0, list.Items().Size()));
    }

    Windows::Foundation::IAsyncAction MainWindow::ApplyClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto strong_this{ get_strong() };

        //strong_this->RenamingPane().Visibility(Visibility::Visible);
        //strong_this->RenamingBar().IsIndeterminate(true);
        /*if (auto mainPage{ find_ancestor<xmtmui::MainWindow>(*strong_this) })
        {
            mainPage.IsEnabled(false);
        }
        else
        {
            strong_this->ControlList().IsEnabled(false);
        }*/

        if (auto items{ strong_this->m_explorerItems })
        {
            const auto selectedCount = strong_this->m_selectedIndex.size();

            std::vector<std::tuple<uint32_t, std::wstring>> errors;
            std::vector<uint32_t> toRemoves;
            errors.reserve(selectedCount);
            toRemoves.reserve(selectedCount);

            co_await[=, &errors, &toRemoves]() -> Windows::Foundation::IAsyncAction
            {
                using folder_t = std::tuple<std::wstring, std::wstring, uint32_t>;

                /*strong_this->RenamingBar().Maximum((double)selectedCount);
                strong_this->RenamingBar().Value(0.0);
                strong_this->RenamingBar().IsIndeterminate(false);*/

                std::vector<folder_t> folders;
                folders.reserve(selectedCount);

                co_await resume_background();

                auto RenameImpl = [&errors, &toRemoves](std::wstring const renamed, fs::path const oldPath, uint32_t index)
                {
                    const auto newpath = oldPath.parent_path() / renamed;

                    std::error_code errcode;
                    fs::rename(oldPath, newpath, errcode);
                    if (errcode)
                    {
                        const auto errmes = Xellanix::Utilities::s_to_ws(errcode.message());
                        errors.emplace_back(index, errmes);
                    }
                    else
                    {
                        toRemoves.emplace_back(index);
                    }
                };

                for (auto& index : strong_this->m_selectedIndex)
                {
                    const auto&& item = items.GetAt(index).try_as<QuickRename::ExplorerItem>();
                    auto const oldpath = fs::path(std::wstring(item.ItemPath()));
                    auto const renamed = std::wstring(item.ItemRenamed());

                    if (fs::is_directory(oldpath))
                    {
                        folders.emplace_back(oldpath.wstring(), renamed, index);
                    }
                    else
                    {
                        RenameImpl(renamed, oldpath, index);

                        /*if (auto bar{ strong_this->RenamingBar() })
                        {
                            co_await wil::resume_foreground(bar.Dispatcher());

                            bar.Value(bar.Value() + 1);

                            co_await resume_background();
                        }*/
                    }
                }

                folders.shrink_to_fit();
                std::sort(folders.begin(), folders.end(), [](folder_t const& lhs, folder_t const& rhs)
                {
                    return natural_less(std::get<0>(lhs), std::get<0>(rhs));
                });
                for (auto rit = folders.rbegin(); rit != folders.rend(); ++rit)
                {
                    const auto& [oldpath, renamed, index] = (*rit);

                    RenameImpl(renamed, fs::path(oldpath), index);

                    /*if (auto bar{ strong_this->RenamingBar() })
                    {
                        co_await wil::resume_foreground(bar.Dispatcher());

                        bar.Value(bar.Value() + 1);

                        co_await resume_background();
                    }*/
                }
            }();

            errors.shrink_to_fit();
            toRemoves.shrink_to_fit();
            std::sort(toRemoves.begin(), toRemoves.end(), std::greater<uint32_t>());

            if (errors.size() > 0)
            {
                ContentDialog dialog{};
                dialog.XamlRoot(strong_this->m_rootElement.XamlRoot());
                dialog.CloseButtonText(L"Close");
                dialog.DefaultButton(ContentDialogButton::Close);
                dialog.Title(box_value(L"Oh No, There Are Errors That Occur During the Rename Process!"));

                StackPanel dialogContent{};
                {
                    TextBlock message{};
                    message.Text(L"There is some additional information about the errors that occurred, which may be helpful in resolving the errors.");
                    message.TextWrapping(TextWrapping::WrapWholeWords);

                    dialogContent.Children().Append(message);
                }

                ScrollViewer scrollView{};
                scrollView.HorizontalScrollBarVisibility(ScrollBarVisibility::Disabled);
                scrollView.HorizontalScrollMode(ScrollMode::Disabled);
                scrollView.VerticalScrollBarVisibility(ScrollBarVisibility::Auto);
                scrollView.VerticalScrollMode(ScrollMode::Enabled);
                scrollView.Margin(ThicknessHelper::FromLengths(0.0, 20.0, 0.0, 0.0));
                TextBlock txtb{};
                txtb.TextWrapping(TextWrapping::WrapWholeWords);

                auto isRemember = strong_this->m_rememberRenamed;
                if (isRemember)
                    strong_this->m_rememberItems.reserve(errors.size());

                for (size_t i = 0; i < errors.size(); i++)
                {
                    auto const& [_index, _mes] = errors[i];
                    std::wstring _path;
                    {
                        auto _item = items.GetAt(_index).try_as<QuickRename::ExplorerItem>();
                        _path = _item.ItemPath();

                        _item.IsRemember(isRemember);
                        if (isRemember)
                            strong_this->m_rememberItems.emplace_back(fs::path(_path).parent_path() / std::wstring(_item.ItemRenamed()));
                    }
                    Documents::Run run1{}, run2{};
                    run1.Text(L"\"" + _path + L"\"");
                    run1.FontWeight(Microsoft::UI::Text::FontWeights::SemiBold());
                    run2.Text(L" >> " + _mes);

                    txtb.Inlines().Append(run1);
                    txtb.Inlines().Append(run2);

                    if (i + 1 == errors.size()) continue;

                    txtb.Inlines().Append(Documents::LineBreak{});
                    txtb.Inlines().Append(Documents::LineBreak{});
                }

                scrollView.Content(txtb);
                dialogContent.Children().Append(scrollView);
                dialog.Content(dialogContent);

                co_await dialog.ShowAsync();
            }

            if (auto filelist{ strong_this->FileList() })
            {
                filelist.DeselectRange(Data::ItemIndexRange(0, filelist.Items().Size()));
            }
            for (auto& i : toRemoves)
            {
                strong_this->m_explorerItemsSet.erase(strong_this->m_explorerItems.GetAt(i).try_as<QuickRename::ExplorerItem>().ItemPath().c_str());
                strong_this->m_explorerItems.RemoveAt(i);
            }
        }

        /*if (auto mainPage{ find_ancestor<xmtmui::MainWindow>(*strong_this) })
        {
            mainPage.IsEnabled(true);
        }
        else
        {
            strong_this->ControlList().IsEnabled(true);
        }*/

        /*strong_this->RenamingBar().IsIndeterminate(false);
        strong_this->RenamingPane().Visibility(Visibility::Collapsed);*/
    }

    #pragma endregion

    #pragma region Settings & Options

    void MainWindow::RecursiveBehavChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& /*e*/)
    {
        SettingInitializer(m_recursiveBehav, RecursiveBehav().SelectedIndex(), L"95c41af7-cd4a-5d5a-b08c-24ce60cfb55b");
    }

    void MainWindow::AutoSortChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        SettingInitializer(m_autoSort, AutoSort().IsOn(), L"1102f064-e390-5331-bd67-760f6fe1c487");
    }

    void MainWindow::RememberRenamedChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        SettingInitializer(m_rememberRenamed, RememberRenamed().IsOn(), L"a698a7d1-be04-54f2-8c62-f3414f5294ee");
    }

    void MainWindow::ResetRememberClicked(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (m_rememberItems.size() == 0) return;

        for (uint32_t i = 0; i < m_explorerItems.Size(); i++)
        {
            if (auto item{ m_explorerItems.GetAt(i).try_as<QuickRename::ExplorerItem>() })
            {
                item.IsRemember(false);
            }
        }

        swapreset(m_rememberItems);

        setNewNameOps.Replace(SetNewName());
        return;
    }

    void MainWindow::RenameRepTextChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        bool _ = RenameRepText().IsChecked().operator std::optional<bool>().value_or(false);

        SettingInitializer(m_renameRepText, _, L"b65d17ad-229c-5b56-8d54-96f70b60665b");
        setNewNameOps.Replace(SetNewName());
    }

    void MainWindow::MatchWholeTextChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        bool _ = MatchWholeText().IsChecked().operator std::optional<bool>().value_or(false);

        SettingInitializer(m_matchWhole, _, L"3209a0fa-f2f5-5917-9b40-7639c83a87f9");
        setNewNameOps.Replace(SetNewName());
    }

    void MainWindow::MatchCaseChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        bool _ = MatchCase().IsChecked().operator std::optional<bool>().value_or(false);

        SettingInitializer(m_matchCase, _, L"e1e7dd78-1dd8-566e-84a8-3cc91e5514a7");
        setNewNameOps.Replace(SetNewName());
    }

    void MainWindow::ApplyToChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& /*e*/)
    {
        SettingInitializer(m_applyTo, ApplyTo().SelectedIndex(), L"21edaabb-210d-5dba-92fe-e90c95ae80e9");
        setNewNameOps.Replace(SetNewName());
    }

    void MainWindow::SpecialFormattingCheckChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) return;

        setNewNameOps.Replace(SetNewName());
    }

    void MainWindow::TextToFind_KeyDown(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e)
    {
        if (e.Key() == Windows::System::VirtualKey::Enter)
        {
            using namespace Microsoft::UI::Xaml::Input;

            FindNextElementOptions option;
            option.SearchRoot(m_rootElement.XamlRoot().Content());
            FocusManager::TryMoveFocus(FocusNavigationDirection::Next, option);

            // Make sure to set the Handled to true, otherwise the RoutedEvent might fire twice
            e.Handled(true);
        }
    }

    void MainWindow::TextToFind_LostFocus(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) return;

        if (auto control{ TextToFind() })
        {
            auto tempText = control.Text();
            if (m_textToFind != tempText)
                m_textToFind = tempText;
            else return;
        }

        setNewNameOps.Replace(SetNewName());
    }

    void MainWindow::NewNameFormat_KeyDown(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e)
    {
        if (e.Key() == Windows::System::VirtualKey::Enter)
        {
            using namespace Microsoft::UI::Xaml::Input;

            FindNextElementOptions option;
            option.SearchRoot(m_rootElement.XamlRoot().Content());
            FocusManager::TryMoveFocus(FocusNavigationDirection::Next, option);

            // Make sure to set the Handled to true, otherwise the RoutedEvent might fire twice
            e.Handled(true);
        }
    }

    void MainWindow::NewNameFormat_LostFocus(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (!isloaded) return;

        if (auto control{ NewNameFormat() })
        {
            auto tempText = control.Text();
            if (m_newNameFormat != tempText)
                m_newNameFormat = tempText;
            else return;
        }

        if (!m_newNameFormat.empty())
        {
            // For '\' character, remove it GenerateNewName function
            std::wstring notSupported{ L"/:*\"<>|" };

            for (auto beginOf = m_newNameFormat.find_first_of(notSupported); beginOf != std::wstring::npos; beginOf = m_newNameFormat.find_first_of(notSupported))
            {
                const auto endOf = m_newNameFormat.find_first_not_of(notSupported, beginOf);

                const auto range = endOf - beginOf;

                m_newNameFormat.erase(beginOf, range);
            }
        }

        setNewNameOps.Replace(SetNewName());
    }

    #pragma endregion

    #pragma region Name Manager

    std::wstring RandomCharacter(std::wstring_view const availableChars = L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
    {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<unsigned long long> dis(0ULL, (availableChars.size() - 1));

        return std::wstring(1, availableChars[dis(gen)]);
    }
    std::wstring RandomLowerCharacter()
    {
        return RandomCharacter(L"abcdefghijklmnopqrstuvwxyz");
    }
    std::wstring RandomUpperCharacter()
    {
        return RandomCharacter(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    }

    bool IsRegexValid(const std::wstring& regex, std::regex_constants::syntax_option_type const& option)
    {
        try
        {
            std::wregex re(regex, option);
        }
        catch (const std::regex_error&)
        {
            return false;
        }
        return true;
    }
    
    bool MainWindow::IsRestartName(hstring const& finalpath)
    {
        if (Xellanix::Utilities::CheckExist(finalpath))
        {
            return true;
        }

        for (auto const& i : m_rememberItems)
        {
            if (i == finalpath)
            {
                return true;
            }
        }
        return false;
    }

    #pragma endregion

    #pragma region Items Manager

    void MainWindow::SortItems(std::vector<hstring>& itemList)
    {
        if (m_autoSort)
        {
            std::sort(itemList.begin(), itemList.end(), [](hstring const& lo, hstring const& ro)
            {
                const std::wstring lhs{ lo };
                const std::wstring rhs{ ro };

                return (fs::is_directory(lhs) > fs::is_directory(rhs)) ||
                    ((fs::is_directory(lhs) == fs::is_directory(rhs)) && (natural_less(lhs, rhs)));
            });
        }
    }

    Windows::Foundation::IAsyncAction MainWindow::AddExplorerItem(winrt::param::hstring const& filepath)
    {
        auto strong_this{ get_strong() };

        fs::path fpath{ filepath.operator const winrt::hstring&().c_str() };

        if (!fpath.has_parent_path() || fpath.parent_path() == fpath)
        {
            co_return;
        }

        if (strong_this->m_explorerItemsSet.count(fpath.wstring()) > 0)
        {
            co_return;
        }

        if (Xellanix::Utilities::CheckExist(fpath))
        {
            auto fname = get_filename(fpath.wstring());
            bool isDir = fs::is_directory(fpath);

            hstring ficon{ isDir ? ExplorerItem_FolderIcon : ExplorerItem_FileIcon };

            auto newItem = winrt::make<implementation::ExplorerItem>(fname, hstring(fpath.wstring()), ficon, L"", false);
            strong_this->m_explorerItems.Append(newItem);
            strong_this->m_explorerItemsSet.insert(fpath.wstring());

            if (isDir)
            {
                // Add recursively or not?
                if (strong_this->m_recursiveBehav == 0ui16) co_return;

                bool isRecursive = false;
                if (strong_this->m_recursiveBehav == 1ui16)
                {
                    ContentDialog dialog{};
                    dialog.XamlRoot(strong_this->m_rootElement.XamlRoot());
                    dialog.PrimaryButtonText(L"Add");
                    dialog.CloseButtonText(L"No Need");
                    dialog.DefaultButton(ContentDialogButton::Primary);
                    dialog.Title(box_value(L"Add All Files in Folder?"));
                    {
                        TextBlock txtb{};
                        Documents::Run run1{}, run2{}, run3{};
                        Documents::Run bold1{}, bold2{};
                        run1.Text(L"Do you want to add all the files in ");
                        bold1.Text(L"\"" + fpath.wstring() + L"\"");
                        bold1.FontWeight(Microsoft::UI::Text::FontWeights::SemiBold());
                        run2.Text(L" as well to rename? Select ");
                        bold2.Text(L"No Need");
                        bold2.FontWeight(Microsoft::UI::Text::FontWeights::SemiBold());
                        run3.Text(L" to add only that folder.");

                        txtb.Inlines().Append(run1);
                        txtb.Inlines().Append(bold1);
                        txtb.Inlines().Append(run2);
                        txtb.Inlines().Append(bold2);
                        txtb.Inlines().Append(run3);

                        txtb.TextWrapping(TextWrapping::WrapWholeWords);
                        dialog.Content(txtb);
                    }

                    auto op{ co_await dialog.ShowAsync() };
                    if (op == ContentDialogResult::Primary)
                    {
                        isRecursive = true;
                    }
                    else
                    {
                        isRecursive = false;
                    }
                }
                else if (strong_this->m_recursiveBehav == 2ui16)
                {
                    if (!strong_this->m_recursiveSetted)
                    {
                        ContentDialog dialog{};
                        dialog.XamlRoot(strong_this->m_rootElement.XamlRoot());
                        dialog.PrimaryButtonText(L"Add");
                        dialog.CloseButtonText(L"No Need");
                        dialog.DefaultButton(ContentDialogButton::Primary);
                        dialog.Title(box_value(L"Add All Files in Folder?"));
                        {
                            TextBlock txtb{};
                            Documents::Run run1{}, run2{};
                            Documents::Run bold1{};
                            run1.Text(L"Do you want to add all the files that are in those folders as well to rename? Select ");
                            bold1.Text(L"No Need");
                            bold1.FontWeight(Microsoft::UI::Text::FontWeights::SemiBold());
                            run2.Text(L" to add only those folders.");

                            txtb.Inlines().Append(run1);
                            txtb.Inlines().Append(bold1);
                            txtb.Inlines().Append(run2);

                            txtb.TextWrapping(TextWrapping::WrapWholeWords);
                            dialog.Content(txtb);
                        }

                        auto op{ co_await dialog.ShowAsync() };
                        if (op == ContentDialogResult::Primary)
                        {
                            strong_this->m_isRecursive = true;
                        }
                        else
                        {
                            strong_this->m_isRecursive = false;
                        }
                    }

                    isRecursive = strong_this->m_isRecursive;
                }
                else if (strong_this->m_recursiveBehav == 3ui16)
                {
                    isRecursive = true;
                }

                if (!isRecursive) co_return;

                std::vector<fs::directory_entry> recurPaths;
                std::copy(fs::recursive_directory_iterator(fpath), fs::recursive_directory_iterator(), std::back_inserter(recurPaths));
                if (strong_this->m_autoSort)
                {
                    std::sort(recurPaths.begin(), recurPaths.end(), [](fs::directory_entry const& lhs, fs::directory_entry const& rhs)
                    {
                        return natural_less(lhs.path().wstring(), rhs.path().wstring());
                    });
                }
                for (auto& entry : recurPaths)
                {
                    AddFromRecursive(entry.path().wstring());
                }
            }
        }
    }

    void MainWindow::AddFromRecursive(winrt::param::hstring const& filepath)
    {
        fs::path fpath{ filepath.operator const winrt::hstring & ().c_str() };

        if (!fpath.has_parent_path() || fpath.parent_path() == fpath)
        {
            return;
        }

        if (m_explorerItemsSet.count(fpath.wstring()) > 0)
        {
            return;
        }

        if (Xellanix::Utilities::CheckExist(fpath))
        {
            auto fname = get_filename(fpath.wstring());
            hstring ficon{ fs::is_directory(fpath) ? ExplorerItem_FolderIcon : ExplorerItem_FileIcon };

            auto newItem = winrt::make<implementation::ExplorerItem>(fname, hstring(fpath.wstring()), ficon, L"", false);
            m_explorerItems.Append(newItem);
            m_explorerItemsSet.insert(fpath.wstring());
        }
    }

    Windows::Foundation::IAsyncAction MainWindow::ShowFilesPicker()
    {
        auto strong_this{ get_strong() };

        Windows::Storage::Pickers::FileOpenPicker picker{};
        picker.as<Xellanix::Desktop::IInitializeWithWindow>()->Initialize(Xellanix::Desktop::WindowHandle);
        picker.FileTypeFilter().Append(L"*");
        auto&& files{ co_await picker.PickMultipleFilesAsync() };

        if (!files) co_return;

        strong_this->m_recursiveSetted = false;
        std::vector<hstring> temp;
        temp.reserve(files.Size());
        for (auto&& file : files)
        {
            temp.emplace_back(file.Path());
        }
        temp.shrink_to_fit();

        strong_this->SortItems(temp);
        for (auto const& file : temp)
        {
            co_await strong_this->AddExplorerItem(file);
        }
    }

    hstring CombinePaths(std::wstring_view const parent, std::wstring_view const current)
    {
        // set *this to the path lexically resolved by _Other relative to *this
        // examples:
        // path{"cat"} / "c:/dog"; // yields "c:/dog"
        // path{"cat"} / "c:"; // yields "c:"
        // path{"c:"} / ""; // yields "c:"
        // path{"c:cat"} / "/dog"; // yields "c:/dog"
        // path{"c:cat"} / "c:dog"; // yields "c:cat/dog"
        // path{"c:cat"} / "d:dog"; // yields "d:dog"
        // several places herein quote the standard, but the standard's variable p is replaced with _Other

        std::wstring _Text{ parent };

        auto is_absolute = [](std::wstring_view const _Text) -> bool
        {
            // paths with a root-name that is a drive letter and no root-directory are drive relative, such as x:example
            // paths with no root-name or root-directory are relative, such as example
            // paths with no root-name but a root-directory are root relative, such as \example
            // all other paths are absolute
            const auto _First = _Text.data();
            const auto _Last = _First + _Text.size();
            if (fs::_Has_drive_letter_prefix(_First, _Last))
            { // test for X:\ but not X:cat
                return _Last - _First >= 3 && std::_Is_slash(_First[2]);
            }

            // if root-name is otherwise nonempty, then it must be one of the always-absolute prefixes like
            // \\?\ or \\server, so the path is absolute. Otherwise it is relative.
            return _First != fs::_Find_root_name_end(_First, _Last);
        };

        if (is_absolute(current))
        {
            // if _Other.is_absolute(), then op=(_Other)
            return hstring(current);
        }

        const auto _My_first = _Text.data();
        const auto _My_last = _My_first + _Text.size();
        const auto _Other_first = current.data();
        const auto _Other_last = _Other_first + current.size();
        const auto _My_root_name_end = fs::_Find_root_name_end(_My_first, _My_last);
        const auto _Other_root_name_end = fs::_Find_root_name_end(_Other_first, _Other_last);
        if (_Other_first != _Other_root_name_end
            && fs::_Range_compare(_My_first, _My_root_name_end, _Other_first, _Other_root_name_end) != 0)
        {
            // if _Other.has_root_name() && _Other.root_name() != root_name(), then op=(_Other)
            return hstring(current);
        }

        if (_Other_root_name_end != _Other_last && std::_Is_slash(*_Other_root_name_end))
        {
            // If _Other.has_root_directory() removes any root directory and relative-path from *this
            _Text.erase(static_cast<size_t>(_My_root_name_end - _My_first));
        }
        else
        {
            // Otherwise, if (!has_root_directory && is_absolute) || has_filename appends path::preferred_separator
            if (_My_root_name_end == _My_last)
            {
                // Here, !has_root_directory && !has_filename
                // Going through our root_name kinds:
                // X: can't be absolute here, since those paths are absolute only when has_root_directory
                // \\?\ can't exist without has_root_directory
                // \\server can be absolute here
                if (_My_root_name_end - _My_first >= 3)
                {
                    _Text.push_back(fs::path::preferred_separator);
                }
            }
            else
            {
                // Here, has_root_directory || has_filename
                // If there is a trailing slash, the trailing slash might be part of root_directory.
                // If it is, has_root_directory && !has_filename, so the test fails.
                // If there is a trailing slash not part of root_directory, then !has_filename, so only
                // (!has_root_directory && is_absolute) remains
                // Going through our root_name kinds:
                // X:cat\ needs a root_directory to be absolute
                // \\server\cat must have a root_directory to exist with a relative_path
                // \\?\ must have a root_directory to exist
                // As a result, the test fails if there is a trailing slash.
                // If there is no trailing slash, then has_filename, so the test passes.
                // Therefore, the test passes if and only if there is no trailing slash.
                if (!std::_Is_slash(_My_last[-1]))
                {
                    _Text.push_back(fs::path::preferred_separator);
                }
            }
        }

        // Then appends the native format pathname of _Other, omitting any root-name from its generic format
        // pathname, to the native format pathname.
        _Text.append(_Other_root_name_end, static_cast<size_t>(_Other_last - _Other_root_name_end));
        return hstring(_Text);;
    }
    
    Windows::Foundation::IAsyncOperation<hstring> MainWindow::GenerateNewName(hstring const& itemName, hstring const& itemPath, bool usespecialsymbols, uint32_t id, bool useregex, std::unordered_multiset<hstring>& renamed_paths)
    {
        co_await resume_background();

        auto const [nameonly, extonly] = split_filename(itemName);
        hstring renamed{ m_applyTo == 2 ? itemName :
            m_applyTo == 1 ? extonly :
            nameonly };

        const auto parent_p = fs::_Parse_parent_path(itemPath.c_str());
        double posibilities;

        do
        {
            posibilities = 0.0;

            std::wstring newnameformat{ m_newNameFormat };
            if (usespecialsymbols)
            {
                const auto _time = Xellanix::Utilities::FileDateCreated(itemPath);
                unsigned long long contain;

                newnameformat = ReplaceIndexSymbol(newnameformat, id);

                contain = 0;

                newnameformat = ReplaceSymbol(newnameformat, LR"ss(@)ss", CALL_WITH_DEF(RandomCharacter), contain);
                newnameformat = ReplaceSymbol(newnameformat, LR"ss(\a)ss", RandomLowerCharacter, contain);
                newnameformat = ReplaceSymbol(newnameformat, LR"ss(\A)ss", RandomUpperCharacter, contain);
                posibilities = std::pow(26ULL, contain);
                contain = 0;

                newnameformat = ReplaceTimeSymbol(newnameformat, LR"ss(h)ss", _time);
                newnameformat = ReplaceTimeSymbol(newnameformat, LR"ss(m)ss", _time);
                newnameformat = ReplaceTimeSymbol(newnameformat, LR"ss(s)ss", _time);
                newnameformat = ReplaceTimeSymbol(newnameformat, LR"ss(d)ss", _time);
                newnameformat = ReplaceTimeSymbol(newnameformat, LR"ss(M)ss", _time);
                newnameformat = ReplaceTimeSymbol(newnameformat, LR"ss(yy)ss", _time);

                // Remove any leaving '\' characters
                for (auto beginOf = newnameformat.find_first_of(L"\\"); beginOf != std::wstring::npos; beginOf = newnameformat.find_first_of(L"\\"))
                {
                    const auto endOf = newnameformat.find_first_not_of(L"\\", beginOf);

                    const auto range = endOf - beginOf;

                    newnameformat.erase(beginOf, range);
                }
            }

            if (m_renameRepText)
            {
                renamed = hstring(m_applyTo == 2 ? itemName :
                                       m_applyTo == 1 ? extonly :
                                       nameonly);

                if (useregex)
                {
                    auto regexSetting = std::regex_constants::ECMAScript;
                    if (m_matchCase) regexSetting |= std::regex_constants::icase;

                    if (IsRegexValid(m_textToFind, regexSetting))
                    {
                        std::wregex reg(m_textToFind, regexSetting);
                        renamed = std::regex_replace(renamed.c_str(), reg, newnameformat);
                    }
                }
                else
                {
                    // Dont use regex!
                    // Use ReplaceAll instead
                    // Find 'tofind' in 'renamed' and replace it with 'newnameformat'
                    renamed = ReplaceAll(renamed.c_str(), m_matchWhole, m_matchCase, m_textToFind, newnameformat);
                }
            }
            else
            {
                // Replace all content of 'renamed' without looking what, with 'newnameformat'
                renamed = newnameformat;
            }

            if (m_applyTo == 1)
            {
                renamed = nameonly + renamed;
            }
            else if (m_applyTo == 0)
            {
                renamed = renamed + extonly;
            }

            size_t same_count;
            {
                auto _finalPath = CombinePaths(parent_p, renamed);
                renamed_paths.insert(_finalPath);

                same_count = renamed_paths.count(_finalPath);
            }
            if (same_count > 1)
            {
                if (id >= posibilities)
                {
                    renamed = add_before_extension(renamed, L" " + to_hstring(same_count));
                }
            }

            if (renamed == itemName)
            {
                break;
            }
        } while (IsRestartName(CombinePaths(parent_p, renamed)));

        co_return renamed;
    }
    
    Windows::Foundation::IAsyncAction MainWindow::SetNewName()
    {
        auto cancellation = co_await get_cancellation_token();
        cancellation.enable_propagation();

        auto strong{ get_strong() };

        auto itemsSize = strong->m_explorerItems.Size();
        using list_size = decltype(itemsSize);

        list_size id = 1;
        std::unordered_multiset<hstring> renamed_paths;
        renamed_paths.reserve(itemsSize);

        for (list_size i = 0; i < itemsSize; i++)
        {
            if (cancellation()) co_return;

            auto&& item = strong->m_explorerItems.GetAt(i).try_as<QuickRename::ExplorerItem>();
            if (item.IsRemember()) continue;

            if (strong->m_selectedIndex.count(i) == 0)
            {
                // not selected!
                item.ItemRenamed(L"");
            }
            else
            {
                // selected!
                hstring renamed = item.ItemName();
                if (!strong->m_textToFind.empty() || !strong->m_renameRepText)
                {
                    renamed = co_await strong->GenerateNewName(item.ItemName(),
                                                               item.ItemPath(),
                                                               strong->UseSpecialSymbols().IsChecked().Value(),
                                                               id,
                                                               strong->UseRegex().IsChecked().Value(),
                                                               renamed_paths);
                }

                item.ItemRenamed(renamed);
                ++id;
            }
        }
    }

    #pragma endregion

    #pragma region File Settings

    template<typename _To, typename _From>
    bool MainWindow::SettingInitializer(_To& to, _From from, std::wstring_view const key)
    {
        if (!isloaded) return false;

        if constexpr (std::is_same_v<_To, _From>)
        {
            to = from;
        }
        else
        {
            to = static_cast<_To>(from);
        }
        
        SaveSettings(key, to);
        return true;
    }
    
    void MainWindow::LoadSettings()
    {
        isloaded = false;

        if (settings.Read(Xellanix::Utilities::LocalAppData / L"Settings\\QuickRename.xsmf"))
        {
            try
            {
                m_recursiveBehav = (settings >> L"95c41af7-cd4a-5d5a-b08c-24ce60cfb55b").try_as<unsigned short>(2);
                m_autoSort = (settings >> L"1102f064-e390-5331-bd67-760f6fe1c487").try_as<bool>(true);
                m_rememberRenamed = (settings >> L"a698a7d1-be04-54f2-8c62-f3414f5294ee").try_as<bool>(true);
                m_renameRepText = (settings >> L"b65d17ad-229c-5b56-8d54-96f70b60665b").try_as<bool>(true);
                m_matchWhole = (settings >> L"3209a0fa-f2f5-5917-9b40-7639c83a87f9").try_as<bool>(false);
                m_matchCase = (settings >> L"e1e7dd78-1dd8-566e-84a8-3cc91e5514a7").try_as<bool>(false);
                m_applyTo = (settings >> L"21edaabb-210d-5dba-92fe-e90c95ae80e9").try_as<unsigned short>(0);
            }
            catch (...)
            {
            }

            RecursiveBehav().SelectedIndex(m_recursiveBehav);
            AutoSort().IsOn(m_autoSort);
            RememberRenamed().IsOn(m_rememberRenamed);
            RenameRepText().IsChecked(m_renameRepText);
            MatchWholeText().IsChecked(m_matchWhole);
            MatchCase().IsChecked(m_matchCase);
            ApplyTo().SelectedIndex(m_applyTo);
        }
        else
        {
            settings[L"95c41af7-cd4a-5d5a-b08c-24ce60cfb55b"] = m_recursiveBehav;
            settings[L"1102f064-e390-5331-bd67-760f6fe1c487"] = m_autoSort;
            settings[L"a698a7d1-be04-54f2-8c62-f3414f5294ee"] = m_rememberRenamed;
            settings[L"b65d17ad-229c-5b56-8d54-96f70b60665b"] = m_renameRepText;
            settings[L"3209a0fa-f2f5-5917-9b40-7639c83a87f9"] = m_matchWhole;
            settings[L"e1e7dd78-1dd8-566e-84a8-3cc91e5514a7"] = m_matchCase;
            settings[L"21edaabb-210d-5dba-92fe-e90c95ae80e9"] = m_applyTo;
        }

        isloaded = true;
    }

    template<typename T>
    inline std::enable_if_t<Xellanix::Objects::is_xsmf_supported_v<T>>
        MainWindow::SaveSettings(std::wstring_view const name, T const& value)
    {
        settings[std::wstring(name)] = value;

        settings.Write(Xellanix::Utilities::LocalAppData / L"Settings\\QuickRename.xsmf");
        get_ftime(L"Settings\\QuickRename.xsmf", last_ctime1, last_mtime1, last_size1);
    }

    #pragma endregion
}

