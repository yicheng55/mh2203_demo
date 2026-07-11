# 行為原則（Karpathy 啟發）

- **編碼前思考** — 不確定就問，不要默默猜；呈現權衡而非隱藏困惑
- **簡潔優先** — 用最少程式碼解決問題，不要為不存在的情境做抽象
- **精準修改** — 只碰必須碰的，只清理自己造成的混亂；每一行修改應能追溯到使用者請求
- **目標驅動執行** — 定義可驗證的成功標準，循環直到達成

# mh2030_demo — 倉庫指引

## 專案概要
以後都使用繁體中文回答!

Davicom MH2203 (ARM Cortex-M0) + DM9051A SPI 乙太網路驅動 SDK 與 demo。
C99，Keil MDK uVision 5 (`.uvprojx`)。非 Makefile/CMake。

## 建置方式

三個 Keil 專案位於 `ModuleDemo/DM9051A/USER/`：

| 專案檔 | 用途 |
|---|---|
| `DM9051A.uvprojx` | 裸 DM9051 功能測試（無 TCP/IP），2 個 target |
| `DM9051A_uip.uvprojx` | uIP + DM9051，3 個 target |
| `DM9051A_lwip.uvprojx` | lwIP + DM9051 |

5 個 target 切換關鍵前置定義：`USE_STDPERIPH_DRIVER`、`MH2203_UIP_PORT`、`MH2203_DM9051_SPI_DMA`、`DMPLUG_INT`。

驗證 target 設定：
```powershell
pwsh tools/keil/validate-dm9051-targets.ps1
```

## 專案結構

| 路徑 | 角色 |
|---|---|
| `ModuleDemo/DM9051A/` | **主要開發區** — DM9051 driver + Keil 專案 |
| `ModuleDemo/DM9051A/dm9051_driver/` | **新版分層驅動**（Core → HAL → Port → Adapter） |
| `drivers/dm9051_edriver_v1.6.1a_beta/` | **舊版單檔驅動**（勿編輯，除非指定要修舊版） |
| `middlewares/` | uIP 1.0 + lwIP 2.1.2（上游，已移植） |
| `apps/` | 應用範例（lwIP web server, uIP demo） |
| `Libraries/` | CMSIS + startup + MH20xxLib 週邊 HAL |
| `tools/keil/` | target 驗證腳本 |

## 新版驅動分層

```
Application / uIP / lwIP
        |
Network Stack Adapter  (adapters/uip/, adapters/lwip/)
        |
DM9051 Core Driver    (core/src/, core/inc/)
        |
DM9051 HAL vtable     (hal/inc/dm9051_hal.h)
        |
MH2203 Platform Port (ports/mh2203/ — SPI1, DMA, IRQ, delay, board)
```

## 分層守則

- Core 不直接操作 SPI/GPIO — 所有硬體操作透過 `dm9051_hal_t` vtable
- Adapter 不直接操作暫存器 — 僅呼叫 Core API
- Port 不含協定棧邏輯 — 僅負責 MCU peripheral 與 HAL binding
- Register 定義集中 `dm9051_regs.h`，共用型別集中 `dm9051_types.h`

## 硬體腳位對應

| Signal | Pin |
|---|---|
| CS | PA15 |
| SCK | PB3 |
| MISO | PB4 |
| MOSI | PB5 |
| INT | PF6 / EXTI6 |
| RST | PF7 |

## GitNexus 工作流程

此專案已由 GitNexus 索引（24814 symbols, 39332 relationships, 300 execution flows）。
- 編輯任何符號前**必須**先執行 `impact({target: "...", direction: "upstream"})`
- HIGH/CRITICAL 風險必須警示使用者並等待確認
- 勿用 find-and-replace 重新命名符號 — 使用 `rename`
- Commit 前必須執行 `detect_changes()` 驗證影響範圍
- 索引更新：`node .gitnexus/run.cjs analyze`

其他工作流程技能檔（`.claude/skills/gitnexus/`）：
- `gitnexus-exploring` — 理解架構 / 查詢程式碼流程
- `gitnexus-impact-analysis` — 修改前衝擊分析
- `gitnexus-debugging` — 追蹤 bug / 錯誤
- `gitnexus-refactoring` — 重新命名 / 提取 / 重構
- `gitnexus-pr-review` — 審查 PR

## 參考文件

| 檔案 | 內容 |
|---|---|
| `SOUL.md` | 完整 agent persona、分層邊界規則、除錯檢查清單、uIP adapter API 對應 |
| `CLAUDE.md` | AGENTS.md 的同步鏡像（兩者需保持一致） |
| `readme.md` | 詳盡的中文驅動手冊：init 流程、RX/TX 路徑、移植指南 |
| `docs/dm9051_uip_adapter_analysis.md` | uIP adapter 深度分析（呼叫鏈、資料流、封包複製） |
| `docs/dm9051_lwip_adapter_analysis.md` | lwIP adapter 深度分析 |
| `docs/DM9051_HAL_REFACTOR_PROMPT.md` | HAL 重構計劃與 prompt |
| `ModuleDemo/DM9051A/dm9051_driver/README.md` | 新版驅動佈局文件 |
| `c_naming_convention_clean_code.md` | C 語言命名規範；新增、重構、命名審查與 code review 時使用 |

## 慣例

- C (Keil ARMCC)，無 C++
- AGENTS.md 與 CLAUDE.md 需保持一致（同步鏡像）
- 繁體中文註解與文件
- 命名慣例：`snake_case`，前綴 `dm9051_`、`mh2203_`、`uip_`、`ethernetif_`
- C 程式命名細則依 `c_naming_convention_clean_code.md`；若與既有公開 API 或第三方 SDK 命名衝突，優先保留相容性並先提出風險
- `.uvprojx` 納入版控（但 `.uvguix.*` 和 `.uvoptx` 排除）
- lwIP 為 `NO_SYS=1` 裸機模式，無 RTOS
- `core/` 和 `hal/` 不能引入 uIP / lwIP / MH2203 header
- `adapters/` 不能引入 MH2203 SPI / GPIO / IRQ header
- `DM9051_USE_UIP` 和 `DM9051_USE_LWIP` 不能同時定義

<!-- gitnexus:start -->
# GitNexus — Code Intelligence

This project is indexed by GitNexus as **mh2203_demo** (73048 symbols, 109972 relationships, 300 execution flows). Use the GitNexus MCP tools to understand code, assess impact, and navigate safely.

> Index stale? Run `node .gitnexus/run.cjs analyze` from the project root — it auto-selects an available runner. No `.gitnexus/run.cjs` yet? `npx gitnexus analyze` (npm 11 crash → `npm i -g gitnexus`; #1939).

## Always Do

- **MUST run impact analysis before editing any symbol.** Before modifying a function, class, or method, run `impact({target: "symbolName", direction: "upstream"})` and report the blast radius (direct callers, affected processes, risk level) to the user.
- **MUST run `detect_changes()` before committing** to verify your changes only affect expected symbols and execution flows. For regression review, compare against the default branch: `detect_changes({scope: "compare", base_ref: "master"})`.
- **MUST warn the user** if impact analysis returns HIGH or CRITICAL risk before proceeding with edits.
- When exploring unfamiliar code, use `query({query: "concept"})` to find execution flows instead of grepping. It returns process-grouped results ranked by relevance.
- When you need full context on a specific symbol — callers, callees, which execution flows it participates in — use `context({name: "symbolName"})`.

## Never Do

- NEVER edit a function, class, or method without first running `impact` on it.
- NEVER ignore HIGH or CRITICAL risk warnings from impact analysis.
- NEVER rename symbols with find-and-replace — use `rename` which understands the call graph.
- NEVER commit changes without running `detect_changes()` to check affected scope.

## Resources

| Resource | Use for |
|----------|---------|
| `gitnexus://repo/mh2203_demo/context` | Codebase overview, check index freshness |
| `gitnexus://repo/mh2203_demo/clusters` | All functional areas |
| `gitnexus://repo/mh2203_demo/processes` | All execution flows |
| `gitnexus://repo/mh2203_demo/process/{name}` | Step-by-step execution trace |

## CLI

| Task | Read this skill file |
|------|---------------------|
| Understand architecture / "How does X work?" | `.claude/skills/gitnexus/gitnexus-exploring/SKILL.md` |
| Blast radius / "What breaks if I change X?" | `.claude/skills/gitnexus/gitnexus-impact-analysis/SKILL.md` |
| Trace bugs / "Why is X failing?" | `.claude/skills/gitnexus/gitnexus-debugging/SKILL.md` |
| Rename / extract / split / refactor | `.claude/skills/gitnexus/gitnexus-refactoring/SKILL.md` |
| Tools, resources, schema reference | `.claude/skills/gitnexus/gitnexus-guide/SKILL.md` |
| Index, status, clean, wiki CLI commands | `.claude/skills/gitnexus/gitnexus-cli/SKILL.md` |

<!-- gitnexus:end -->
