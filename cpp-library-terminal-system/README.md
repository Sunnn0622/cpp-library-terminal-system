# 智慧圖書館借閱管理系統

這是一個使用 C++ 製作的終端機 UI 期末專題，主題為圖書館館藏與借閱管理。專案展示 C++ 類別繼承、檔案讀寫、STL 類別庫，以及完整的文字選單操作流程。

## 專題需求對照

| 作業要求 | 本專案實作 |
| --- | --- |
| C++ 類別繼承 | `LibraryItem` 為抽象基底類別，`Book`、`Magazine`、`MediaItem` 繼承並覆寫虛擬函式 |
| 讀檔與寫檔 | 啟動時讀取 `data/library_items.csv`，新增、借出、歸還與離開時寫回檔案 |
| STL 類別庫 | 使用 `vector`、`unique_ptr`、`map`、`sort`、`find_if`、`transform` |
| 終端機 UI | 提供完整選單：查詢、新增、借出、歸還、搜尋、統計 |
| 規格與迭代流程 | 見 `docs/specification.md` 與 `docs/development_log.md` |
| 說明文件 | 本 README 與 `docs/user_manual.md` |

## 編譯方式

```bash
g++ -std=c++17 -Wall -Wextra -pedantic src/main.cpp -o library_system
```

Windows PowerShell 可使用：

```powershell
C:\msys64\ucrt64\bin\g++.exe -std=c++17 -Wall -Wextra -pedantic src\main.cpp -o library_system.exe
```

## 執行方式

請在專案根目錄執行，因為程式會讀寫 `data/library_items.csv`。

```bash
./library_system
```

Windows PowerShell：

```powershell
.\library_system.exe
```

## GitHub Repo

預計網址：

https://github.com/Sunnn0622/cpp-library-terminal-system
