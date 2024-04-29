PalServer.exe 執行後只會顯示一個小黑窗 現階段也沒辦法在黑窗內輸入任何指令
我們可以用Ctrl+C或按X 讓伺服器直接關閉
但在一般邏輯上 這不是一個正常操作方式
根據官方教學文章內容可以知道 伺服器指令只能運用在遊戲內和RCON中
但我覺得就為了關伺服器去開遊戲很麻煩

我自己開發了一個管理工具 可以方便操作PalServer

請在伺服器Config(PalWorldSettings.ini)中開啟RCON(RCONEnabled=True)
RCON預設端口是25575(RCONPort=25575)
你需要設定AdminPossword才能連線RCON

<br />

自1.1.0版本起，請將程式放在steamcmd.exe同級目錄
例：steamcmd.exe在D:\Palworld_SteamCMD\steamcmd.exe
程式則應該放在D:\Palworld_SteamCMD\PalServer Manager.exe
伺服器執行檔會在D:\Palworld_SteamCMD\steamapps\common\PalServer\PalServer.exe
