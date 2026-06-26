---
name: git-commit
description: >-
   依照 mh2030_demo 嵌入式 C / MCU 專案的 Git commit 規範，
   協助撰寫、確認並執行 commit（與可選的 push）。
   Use when: user wants to commit changes, write a commit message, git commit,
   寫 commit、提交、commit message、推版、push。
---

# Git Commit 助手（git-commit）

依照 [COMMIT_CONVENTION.md](COMMIT_CONVENTION.md) 為本專案產生符合規範的 commit message，
並依使用者需求執行 commit / push。

---

## 模式說明

| 模式 | 觸發方式 | 行為 |
|------|----------|------|
| **草稿模式** | 只要求寫 commit msg | 產出草稿供使用者自行執行，不執行任何 git 指令 |
| **半自動模式** | 要求「幫我 commit」 | 顯示草稿 → 使用者確認 → 執行 `git commit`，不 push |
| **全自動模式** | 要求「幫我 commit 並 push」 | 顯示草稿 → 使用者確認 → 執行 `git commit` + `git push` |

> **全自動模式強制確認**：不論使用者說「直接做」或「不用問我」，
> commit message 草稿**一定要讓使用者看到並明確批准後**才能執行 commit 與 push。
> Push 是對共用遠端的不可逆操作，禁止跳過確認步驟。

---

## 執行流程

### Step 1 — 確認 Staging 範圍

先執行 `git status` 與 `git diff --cached --name-only` 取得目前狀態，再依以下情況處理：

**情況 A：使用者在指令中已指定檔案或路徑**

- 僅對指定範圍執行 `git add <paths>`，其他異動檔案保持原狀
- 執行前列出將要 stage 的檔案清單，讓使用者確認

**情況 B：目前已有 staged 變更（且使用者未另外指定）**

- 直接使用現有 staged 內容，不額外 `git add`
- 顯示已 staged 的檔案清單供使用者確認範圍正確

**情況 C：無任何 staged 變更，使用者也未指定檔案**

- 列出所有 unstaged 異動檔案（含路徑），讓使用者選擇：
  1. 指定要 stage 的檔案或目錄（可多個）
  2. 或輸入「全部」代為執行 `git add -A`（需再次確認）
- 不可自行判斷並 stage 所有變更

收集完 staged 範圍後，再執行：

```bash
git diff --cached          # 已 stage 的完整 diff（用於分析 commit 內容）
git log --oneline -5       # 最近 5 筆 commit（用於對齊風格）
```

#### 模組分離判斷

分析 `git diff --cached` 結果是否涉及多個 driver layer：

| 路徑前綴 | 模組名稱 |
|----------|----------|
| `dm9051_driver/core/` | DM9051 Core Driver |
| `dm9051_driver/hal/` | HAL Interface |
| `dm9051_driver/ports/mh2030a/` | MH2030A Platform Port |
| `dm9051_driver/adapters/uip/` | uIP Adapter |
| `dm9051_driver/adapters/lwip/` | lwIP Adapter |
| `middlewares/` | Third-party stack (uIP/lwIP) |
| `apps/` | Application examples |
| `Libraries/` | MCU peripheral library |
| `tools/` | Build / validation scripts |

- 所有變更屬同一層 → 繼續
- 涉及多個 layer → 列出各層變更行數，詢問使用者：
  - 是否要拆分個別 commit？
  - 或維持同一個 commit 一起提交？
- 跨層判斷原則：Core + Port 建議合併（相依性高）；Core + Adapter 建議合併（API 對應）；Core + APP 建議拆分

#### 從 Branch 名稱提取識別

若使用者未提供任何識別碼，從目前分支名稱預設提取（完全選填，僅在確有 issue tracking 時使用）：

- `fix/dm9051-spi-timeout` → 不提取（本專案無強制 issue 綁定）
- 分支名稱中的數字非必要不自動帶入 Footer

---

### Step 2 — 判斷 Component

依 COMMIT_CONVENTION.md 規則選出**唯一**一個 Component：

**優先順序：**
1. 以變更檔案所在目錄決定（見 Step 1 模組對應表）
2. 跨層時以底層為主：`DM9051` > `HAL` > `PORT` > `uIP` / `lwIP` > `APP`
3. 僅改文件 → `DOC`；僅改 Keil `.uvprojx` → `BUILD`

**Component 與檔案的快速對應：**

| 變更檔案位置 | Component |
|---|---|
| `core/src/dm9051_core.c`, `core/inc/dm9051_core.h`, `dm9051_regs.h` | `DM9051` |
| `hal/inc/dm9051_hal.h`, 或新加 `_vtable` 檔案 | `HAL` |
| `ports/mh2030a/` 下任何檔案 | `PORT` |
| `adapters/uip/` 下任何檔案 | `uIP` |
| `adapters/lwip/` 下任何檔案 | `lwIP` |
| `apps/` 下任何檔案 | `APP` |
| `*.md`, `docs/`, html 檔案 | `DOC` |
| `.uvprojx`, `.uvoptx` | `BUILD` |
| `tools/` 下 PowerShell 或 Python 腳本 | `TOOL` |
| `Libraries/` 下 MCU 週邊程式庫 | `LIBS` |
| `.agents/`, `.claude/` 下 skill/workflow | `SKILL` |

---

### Step 3 — 撰寫 Subject

- 格式：`[Component] 英文祈使句（50 字內，不加句號）`
- 必須描述**實際改動內容**，不可泛泛而談
- 全英文（與專案既有 commit 風格一致）

```
# 好的 Subject
[DM9051] Fix SPI DMA timeout on large transfers
[HAL] Add dm9051_hal_bind() for platform port registration
[uIP] Refactor to uip_ethernetif struct with link_poll API
[PORT] Adjust SPI clock prescaler for 20 MHz operation

# 不好的 Subject
[DM9051] Fix bug
[DM9051] Update driver
[DOC] Update doc
```

---

### Step 4 — 判斷 Body（選填）

需要 Body 的情況：
- 標題無法完整說明複雜改動（多個檔案、跨層、破壞性變更）
- 影響多個 Keil target（需說明哪些 target 受影響）
- 需補充驗證方式（如邏輯分析儀測量、throughput 測試結果）
- 新 MCU 平台移植（需說明平台差異）

Body 格式：

```
Detailed changes:
- Implement dm9051_hal_read_mem_dma() using SPI1 DMA channel
- Add DMA completion callback with interrupt flag handling
- Configure DMA transfer size to match DM9051 FIFO granularity

Overall impact and purpose:
Enables zero-copy RX/TX for DM9051 on MH2030A,
reducing CPU load during network throughput tests.
```

- `Detailed changes:` 使用 `- ` bullet point 逐項列出具體改動
- `Overall impact and purpose:` 說明總體目的與影響
- 每行不超過 72 字元

不需 Body 的情況：
- 單一檔案 < 10 行變更
- 純格式/註解/拼字修正
- 單純 Keil target 定義增減

---

### Step 5 — Footer（選填）

本專案**無強制 Footer 制度**，僅在以下情況使用：

```
Related: #123
```

- 有明確的 GitHub Issue 連結時
- 無則省略，不可捏造

---

### Step 6 — 提交前 Review

基於 Step 1 收集的 diff 執行獨立 review。

#### 標準檢查項

- **敏感資訊**：密碼、金鑰、token、私有憑證
- **臨時程式碼**：除錯 printf、TODO 佔位、硬編碼測試資料、註解掉的程式碼
- **衝突與格式**：衝突標記、尾隨空格、縮排異常、不一致的命名風格
- **提交範圍**：不相關檔案、生成物（`.o`、`.hex`、`.bin`）、大檔案誤入

#### 嵌入式 C 特有檢查項

| 檢查項 | 說明 |
|--------|------|
| **Register bitfield** | Mask/shift 數值是否正確、reserved bit 是否意外寫入 |
| **Volatile 正確性** | Memory-mapped register 指標是否正確宣告 `volatile` |
| **Memory alignment** | `uint32_t` 存取是否 4-byte aligned（Cortex-M0 無 unaligned support） |
| **Interrupt safety** | Shared state 變數是否被 critical section 保護、IRQ handler 是否在正確 context 執行 |
| **SPI/DMA timing** | Timeout 值合理性（以 us 為單位）、DMA callback 是否在 IRQ context 中 call |
| **HAL 邊界** | Core/adapter 是否直接操作 MCU 硬體暫存器（必須透過 HAL vtable） |
| **Init order** | 新硬體模組是否在正確順序初始化（DM9051 reset → SPI config → IRQ enable） |
| **Stack impact** | 增加的區域變數/遞迴是否可能造成 stack overflow（Cortex-M0 無 MPU，64KB SRAM） |
| **GitNexus impact** | 修改已索引符號前是否已執行 upstream impact analysis |

#### 處理邏輯

- **無阻塞問題** → 繼續執行 Step 7
- **有阻塞問題** → 列出問題檔案位置與建議修復方式，暫停提交
- **有非阻塞風險** → 繼續 Step 7，但輸出草稿時一併提示風險

---

### Step 7 — 輸出草稿並確認

以下方模板呈現草稿，**等待使用者明確回覆「確認」或修改意見**：

```
────────────────────────────────
Commit Message 草稿
────────────────────────────────
[Component] Subject

Detailed changes:
- Change 1
- Change 2

Overall impact and purpose:
...
────────────────────────────────
模式：[草稿 / commit / commit + push]

請確認內容。若需調整請直接告知，確認後輸入「ok」或「確認」即可執行。
```

注意：專案 commit 風格為純文字為主，不使用 emoji 裝飾。

---

### Step 8 — 執行（半自動 / 全自動模式）

Step 7 使用者確認後：

1. **Commit**：
   ```bash
   git commit -m "$(cat <<'EOF'
   [Component] Subject

   Detailed changes:
   - Change 1
   - Change 2

   Overall impact and purpose:
   ...
   EOF
   )"
   ```
   使用 heredoc 確保多行訊息格式正確，**禁止使用 `--no-verify`**。

2. **Push**（全自動模式）：

   Push 前先執行：
   ```bash
   git log @{u}..HEAD --oneline
   ```
   確認本地領先提交內容符合預期，避免誤推。

   若目前 branch 無 upstream 追蹤，告知使用者並詢問是否要設定 upstream。

   確認後執行：
   ```bash
   git push
   ```

3. 回報執行結果（成功 commit hash / 失敗錯誤訊息）。

---

## 格式自我檢查清單

執行前確認 commit message 符合以下規則：

- [ ] Component 只有一個，來自規範表
- [ ] Subject 格式為 `[Component] Description`，≤ 50 字，祈使句，不加句號
- [ ] Body 與 Subject 之間有空行（若有 Body）
- [ ] Body 使用 `Detailed changes:` + `Overall impact and purpose:` 結構（若有 Body）
- [ ] 沒有捏造 Related Issue 連結
- [ ] 沒有 emoji 或非 ASCII 裝飾

---

## 邊界情況

| 情況 | 處理方式 |
|------|----------|
| 無 staged 變更且未指定檔案 | 列出所有 unstaged 檔案，要求使用者指定範圍後再繼續 |
| 僅改 Keil `.uvprojx` 或 `.uvoptx` | Component 使用 `BUILD` |
| 僅改 `AGENTS.md`、`CLAUDE.md`、`SOUL.md` 或 skill 檔案 | Component 使用 `SKILL` |
| 修改 `dm9051_regs.h` 中的 register 定義 | 影響多個 source file，需先執行 GitNexus impact analysis，在 Body 中說明 register 變更影響 |
| 跨層變更（如 core + port） | Component 以底層為準，Body 說明各層變更 |
| Push 遠端需要驗證或 force-push | 停止並告知，不自動加 `--force` |
| Pre-commit hook 失敗 | 回報錯誤內容，不 amend，協助排查後重新 commit |
| 使用者要修改草稿 | 重新輸出修改後的草稿，再次等待確認 |
| Subagent review 發現阻塞問題 | 列出問題檔案位置與建議修復方式，暫停提交流程 |
| 變更涉及多個 driver layer | 列出各層行數，詢問是否拆分 commit |
| 本地 branch 無 upstream 追蹤 | 告知無法自動 push，詢問是否要設定 upstream |

---

## 參考資源

- 完整規範：[COMMIT_CONVENTION.md](COMMIT_CONVENTION.md)
- 專案分層架構與邊界規則：`SOUL.md`
- GitNexus impact analysis：修改已索引符號前必須執行
