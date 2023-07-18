Add-Type -TypeDefinition @"
using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Automation;
using System.Runtime.InteropServices;
using System.Drawing;
using System.Text.RegularExpressions;

        public class WindowInfo
        {
            public string Title { get; set; }
            public string Process { get; set; }
            public string Url { get; set; }
            public WindowInfo(string title, string process, string url)
            {
                this.Title = title;
                this.Process = process;
                this.Url = url;
            }
        }

        public class GetActiveWindowHelper
        {
            private WindowInfo _currentInfo = new WindowInfo("", "", "");

            public static string GetActiveUrl()
            {
                TreeWalker walker = TreeWalker.ControlViewWalker;
                var found = AutomationElement.FocusedElement;
                try
                {
                    while (found.Current.ControlType != ControlType.Document || !found.GetSupportedPatterns().Contains(ValuePattern.Pattern))
                    {
                        found = walker.GetParent(found);
                        if (found == null) break;
                    }

                    if (found == null) return "";

                    return ((ValuePattern)found.GetCurrentPattern(ValuePattern.Pattern)).Current.Value;
                }
                catch
                {
                    return "";
                }
            }

            public static AutomationElement GetForegroundAutomationElement()
            {
                IntPtr foregroundWindowHandle = GetForegroundWindow();
                if (foregroundWindowHandle != IntPtr.Zero)
                {
                    AutomationElement foregroundElement = AutomationElement.FromHandle(foregroundWindowHandle);
                    return foregroundElement;
                }
                return null;
            }

            public static bool IsWebBrowser(string title)
            {
                return title.Contains("Firefox") || title.Contains("Edge") || title.Contains("Chrome");
            }

            public WindowInfo GetActiveWindow()
            {
                var fg = GetForegroundAutomationElement();
                if (fg != null)
                {
                    try
                    {
                        var title = fg.Current.Name;

                        if (title != this._currentInfo.Title)
                        {
                            var processId = fg.Current.ProcessId;
                            var processName = System.Diagnostics.Process.GetProcessById(processId).ProcessName;
                            var url = (IsWebBrowser(title)) ? GetActiveUrl() : "";
                            this._currentInfo = new WindowInfo(title, processName, url);
                        }
                        else if (IsWebBrowser(title) && this._currentInfo.Url == "")
                        {
                            this._currentInfo.Url = GetActiveUrl();
                        }
                    }
                    catch
                    {
                        return this._currentInfo;
                    }
                }

                return this._currentInfo;
            }


            [System.Runtime.InteropServices.DllImport("user32.dll")]
            public static extern IntPtr GetForegroundWindow();
         }

        public class GetImeHelper
        {
            public class ImeStatus
            {
                public int ConvMode { get; set; }
                public int OpenStatus { get; set; }

                public ImeStatus(int convMode, int openStatus)
                {
                    this.ConvMode = convMode;
                    this.OpenStatus=openStatus;
                }
            }

            public struct GUITHREADINFO
            {
                public int cbSize;
                public int flags;
                public IntPtr hwndActive;
                public IntPtr hwndFocus;
                public IntPtr hwndCapture;
                public IntPtr hwndMenuOwner;
                public IntPtr hwndMoveSize;
                public IntPtr hwndCaret;
                public System.Drawing.Rectangle rcCaret;
            }

            const int WM_IME_CONTROL = 0x0283;
            static readonly IntPtr IMC_GETCONVERSIONMODE = (IntPtr)1;
            static readonly IntPtr IMC_GETOPENSTATUS = (IntPtr)5;

            public static ImeStatus GetImeMode()
            {
                var info = new GUITHREADINFO();
                info.cbSize = System.Runtime.InteropServices.Marshal.SizeOf(info);
                if (GetGUIThreadInfo(0, out info))
                {
                    var imeWnd = ImmGetDefaultIMEWnd(info.hwndFocus);
                    var convMode = SendMessage(imeWnd, WM_IME_CONTROL, IMC_GETCONVERSIONMODE, IntPtr.Zero);
                    var openStatus = SendMessage(imeWnd, WM_IME_CONTROL, IMC_GETOPENSTATUS, IntPtr.Zero);
                    return new ImeStatus((int)convMode, (int)openStatus);
                }
                else
                {
                    return new ImeStatus(0, 0);
                }

            }

            [DllImport("user32.dll")]
            public static extern bool GetGUIThreadInfo(uint idThread, out GUITHREADINFO lpgui);

            [DllImport("imm32.dll")]
            public static extern IntPtr ImmGetDefaultIMEWnd(IntPtr hWnd);


            [DllImport("user32.dll")]
            public static extern IntPtr SendMessage(IntPtr hWnd, uint Msg, IntPtr wParam, IntPtr lParam);

        }

        public class ApplicationMatcher
        {
            private Dictionary<int, WindowInfo> _dict = new Dictionary<int, WindowInfo>();
            public ApplicationMatcher()
            {

            }

            public void AddApplication(int id, string title, string process, string url)
            {
                _dict.Add(id, new WindowInfo(title, process, url));
            }

            public List<int> GetCurrentApplicationIds(WindowInfo current)
            {
                List<int> matchingKeys = _dict.Where(kvp =>
                                    (kvp.Value.Title == "" || Regex.IsMatch(current.Title, kvp.Value.Title)) &&
                                    (kvp.Value.Process == "" || kvp.Value.Process == current.Process) &&
                                    (kvp.Value.Url == "" || Regex.IsMatch(current.Url, kvp.Value.Url))
                                    ).Select(kvp => kvp.Key).ToList();

                return matchingKeys;
            }
        }

"@ -ReferencedAssemblies ("System.Drawing", "UIAutomationClient", "UIAutomationTypes")

Add-Type -AssemblyName System.Windows.Forms
# set global variables
$global:helper = new-Object GetActiveWindowHelper
$global:prevId = @()
$global:prevWin = New-Object PSObject
$global:prevIme = New-Object PSObject
$global:prevWinForBaloon = New-Object PSObject

function CheckActiveWindow($p) {
    # Get active window and ime status
    $win = $global:helper.GetActiveWindow()
    $ime = [GetImeHelper]::GetImeMode()

    # If active window is updated, check application ids
    if (($global:prevWin.Title -ne $win.Title) -or ($global:prevWin.Process -ne $win.Process) -or ($global:prevWin.Url -ne $win.Url)
    ) {
        # Check active window is registed application
        $id = $matcher.GetCurrentApplicationIds($win)
        Write-Host $win.Title $win.Process $win.Url
        $global:prevWin = $win

        if ($global:prevWin.Process -ne "explorer") {
            $global:prevWinForBaloon = $global:prevWin
        }

        # Send new application ID to keyboard quantizer
        if (@(Compare-Object $global:prevId $id).Length -ne 0) {
            $p.WriteLine("activate $id")
            Write-Host("activate $id")
            $global:prevId = $id
        }
    }

    # Update IME Status
    if (($global:prevIme.ConvMode -ne $ime.ConvMode) -or ($global:prevIme.OpenStatus -ne $ime.OpenStatus)) {
        # Write-Host $ime.ConvMode $ime.OpenStatus
        $global:prevIme = $ime
    }
}

# Show file select dialog and return selected file path
function SelectConfigFile($save) {
    $f = New-Object System.Windows.Forms.Form
    $f.TopMost = $true
    if ($save) {
        $dialog = New-Object System.Windows.Forms.SaveFileDialog
    }
    else {
        $dialog = New-Object System.Windows.Forms.OpenFileDialog
    }
    $dialog.Filter = "bin files (*.bin)|*.bin"
    $result = $dialog.ShowDialog($f);
    if ($result -eq 'OK') {
        return $dialog.FileName
    }
    else {
        return $null
    }
}

# Save binary data to file
function SaveReceivedData($data) {
    $file = SelectConfigFile($true)

    if ($null -ne $file) {
        try {
            [System.IO.File]::WriteAllBytes($file, $data)
            Write-Host "Data saved to: $file"
        }
        catch {
            Write-Host $_
            Write-Host "Failed to write"
        }
    }
}

# Backup config file from keyboard quantizer
function backupConfig($p) {
    $p.ReadTimeout = 1000
    $p.ReadExisting()
    # Send backup command
    $p.WriteLine("backup")
    $p.ReadLine()
    Write-Host "backup config file..."

    $notify.BalloonTipText = "Start config file backup"
    $notify.ShowBalloonTip(5000)

    $receivedData = New-Object System.IO.MemoryStream
    $buffer = New-Object byte[] $p.ReadBufferSize
    $stopWatch = New-Object System.Diagnostics.Stopwatch
    $stopWatch.Start()
    # Data receive loop
    while ($true) {
        if ($p.BytesToRead -gt 0) {
            $bytesRead = $p.Read($buffer, 0, $buffer.Length)
            $receivedData.Write($buffer, 0, $bytesRead)
            $stopWatch.Restart()
            Write-Host "receive"
        }
        # If no new data is received while 1s, complete receive loop
        if ($stopWatch.ElapsedMilliseconds -ge 1000) {
            Write-Host "select directory"
            $writeData = $receivedData.GetBuffer()
            SaveReceivedData($writeData[0..($writeData.Length - 3)])
            break
        }
        Start-Sleep -Milliseconds 200
    }
    $notify.BalloonTipText = "Complete config file backup"
    $notify.ShowBalloonTip(5000)
}

# Load config file to keyboard quantizer
function loadConfig($p, $notify) {
    $file = SelectConfigFile($false)

    if ($null -eq $file) {
        Write-Host "No config file is selected"
        return
    }

    # Read config file
    $fileBytes = [System.IO.File]::ReadAllBytes($file)

    try {
        $p.ReadTimeout = 1000
        $p.ReadExisting()
        # Send load command
        $p.WriteLine("load " + $fileBytes.Length)
        $p.ReadLine()

        $notify.BalloonTipText = "Start loading config file"
        $notify.ShowBalloonTip(5000)

        Start-Sleep -Milliseconds 200
        Write-Host "load config file..."
        $startIndex = 0
        $chunkSize = 64

        # Split data in 64bytes chunks and send them with handshake
        while ($startIndex -lt $fileBytes.Length) {
            Write-Host "Send data..."
            $chunk = $fileBytes[$startIndex..($startIndex + $chunkSize - 1)]
            # Send data
            $p.Write($chunk, 0, $chunk.Length)
            $startIndex += $chunkSize
            # Wait hand shake
            Write-Host $p.ReadExisting()
        }

        $notify.BalloonTipText = "Complete loading config file"
        $notify.ShowBalloonTip(5000)
    }
    catch {
        Write-Host $_
        $notify.BalloonTipText = "Failed to load config file"
        $notify.ShowBalloonTip(5000)
    }
}

# Show file select dialog and return selected file path
function SelectFirmwareFile() {
    $f = New-Object System.Windows.Forms.Form
    $f.TopMost = $true
    $dialog = New-Object System.Windows.Forms.OpenFileDialog
    $dialog.Filter = "uf2 files (*.uf2)|*.uf2"
    $result = $dialog.ShowDialog($f);
    if ($result -eq 'OK') {
        return $dialog.FileName
    }
    else {
        return $null
    }
}

function updateFirmware($p, $notify) {
    $targetDrive = "RPI-RP2"

    if (!$p.isOpen) {
        return
    }

    $file = SelectFirmwareFile
    if ($null -eq $file) {
        Write-Host "No config file is selected"
        return
    }
    # $p.ReadTimeout = 1000
    $p.ReadExisting()
    # Send dfu command
    $p.WriteLine("dfu")
    # port may close

    $notify.BalloonTipText = "Start firmware update"
    $notify.ShowBalloonTip(5000)

    # Wait until uf2 drive is recognized
    Start-Sleep -Milliseconds 5000

    $driveQuery = "SELECT * FROM Win32_Volume WHERE DriveType = 2"  
    $drive = Get-WmiObject -Query $driveQuery | Where-Object { $_.Label -eq $targetDrive }

    if ($drive) {
        Write-Host "Drive is found"
        $targetPath = "{0}\{1}" -f $drive.Name, (Split-Path $file -Leaf)
        Copy-Item -Path $file -Destination $targetPath
        $notify.BalloonTipText = "Complete firmware update"
        $notify.ShowBalloonTip(5000)
    }
    else {
        Write-Host "Drive is not found"
    }
}

$appContext = New-Object Windows.Forms.ApplicationContext;

$notify = New-Object System.Windows.Forms.NotifyIcon
$notify.Visible = $true
$notify.Icon = [System.Drawing.Icon]::ExtractAssociatedIcon((Get-Process -id $PID).Path)
$notify.BalloonTipIcon = [System.Windows.Forms.ToolTipIcon]::Info
$notify.BalloonTipTitle = "Keyboard Quantizer"
$notify.BalloonTipText = "Start script"
$notify.ShowBalloonTip(5000)
$notify.add_MouseMove({
        $notify.Text = "Keyboard Quantizer"
    })
$notify.add_Click({
        if ($_.Button -eq "Left") {
            [System.Windows.Forms.MessageBox]::Show("Title: $($global:prevWinForBaloon.Title)`n Process:$($global:prevWinForBaloon.Process)`nURL: $($global:prevWinForBaloon.Url)")
        }
    })

# Open serial port. Port number is set by the first argument
$p = New-Object System.IO.Ports.SerialPort($args[0])
$p.DtrEnable = 1
$p.RtsEnable = 1
$p.ReadTimeout = 1000

$menuExit = New-Object System.Windows.Forms.ToolStripMenuItem
$menuExit.Text = "Exit"
$menuExit.add_Click({ $appContext.ExitThread() })
$menuBackup = New-Object System.Windows.Forms.ToolStripMenuItem
$menuBackup.Text = "Backup config from device"
$menuBackup.add_Click({ try { backupConfig $p $notify } catch { Write-Host $_ } })
$menuLoad = New-Object System.Windows.Forms.ToolStripMenuItem
$menuLoad.Text = "Load config to device"
$menuLoad.add_Click({ try { loadConfig $p $notify } catch { Write-Host $_ } })
$menuUpdate = New-Object System.Windows.Forms.ToolStripMenuItem
$menuUpdate.Text = "Update firmware of device"
$menuUpdate.add_Click({ try { updateFirmware $p $notify } catch { Write-Host $_ } })
$menuSeparator = New-Object System.Windows.Forms.ToolStripSeparator
$menuSeparator2 = New-Object System.Windows.Forms.ToolStripSeparator
$notify.ContextMenuStrip = New-Object System.Windows.Forms.ContextMenuStrip
$notify.ContextMenuStrip.Items.AddRange(($menuBackup, $menuLoad, $menuSeparator, $menuUpdate, $menuSeparator2, $menuExit))
$menuGroup = ($menuBackup, $menuLoad, $menuUpdate)
$menuGroup.ForEach({$_.Enabled = $false})

$matcher = New-Object ApplicationMatcher

$global:reader = New-Object PSObject
$global:portOpen = $false

$timer = New-Object Windows.Forms.Timer;
$timer.add_Tick({
        $timer.Stop()
        $nextInterval = 100

        try {
            CheckActiveWindow($p)
        }
        catch {
        }

        if ($p.IsOpen) {
            $global:portOpen = $true

            try {
                $p.ReadTimeout = 1
                # If some data is received, print it
                $line = $global:reader.ReadLine()
                Write-Host $line
                # If received data is command from keyboard quantizer, execute it
                if ($line -match "^(>?\s*)command:\s*(.+)") {
                    $command = $matches[2]
                    Write-Host $command
                    Invoke-Expression $command
                }
                elseif ($line -match "^(>?\s*)send_string_u:\s*(.+)") {
                    # Send string including unicode characters
                    [System.Windows.Forms.SendKeys]::SendWait($matches[2])
                }
            }
            catch {
                # timeout
            }
        }
        else {
            if ($global:portOpen) {
                Write-Host "Close port"
                $menuGroup.ForEach({$_.Enabled = $false})
                $notify.BalloonTipText = "Serial port is closed"
                $notify.ShowBalloonTip(5000)
            }

            $global:portOpen = $false

            try {
                $p.Open()
                $p.ReadTimeout = 1000

                $stream = $p.BaseStream
                $global:reader = New-Object System.IO.StreamReader($stream, [System.Text.Encoding]::UTF8)

                Write-Host "Open port"
                $menuGroup.ForEach({$_.Enabled = $true})
                $notify.BalloonTipText = "Serial port is opened"
                $notify.ShowBalloonTip(5000)

                # Get application list from keyboard quantier in json format
                $p.WriteLine("") # discard current command
                $p.ReadExisting()
                $p.WriteLine("app")
                $global:reader.ReadLine() # discard echo back
                $line = $global:reader.ReadLine() # get app string
                $apps = $line | ConvertFrom-Json
                Write-Host $apps.app
                # Set applicaton list to application matcher
                for ($i = 0; $i -lt $apps.app.Length; $i++) {
                    $matcher.AddApplication($i, $apps.app[$i].title, $apps.app[$i].process, $apps.app[$i].url)
                }
            }
            catch {
                $nextInterval = 1000
            }
        }

        $timer.Interval = $nextInterval
        $timer.Start()
    })


$timer.Interval = 1
$timer.Start()
[System.Windows.Forms.Application]::Run($appContext);
if ($p.IsOpen) { $p.Close() }
$notify.Dispose()
