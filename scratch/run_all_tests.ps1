$ErrorActionPreference = "Continue"

Write-Host "=================================================================="
Write-Host "         PBL2 - TOÀN BỘ BỘ KIỂM THỬ HỆ THỐNG (FULL TEST SUITE)    "
Write-Host "=================================================================="
Write-Host ""

# 1. Thu thập mã băm SHA256 toàn bộ file trong data/ để giám sát bất biến
$dataFileHashesBefore = @{}
Get-ChildItem -Path "data" -Recurse -File | ForEach-Object {
    $dataFileHashesBefore[$_.FullName] = (Get-FileHash -Path $_.FullName -Algorithm SHA256).Hash
}
Write-Host "[BẢO MẬT DỮ LIỆU] Đã lập chỉ mục SHA256 cho $($dataFileHashesBefore.Count) files trong data/." -ForegroundColor Cyan

# 2. Chuẩn bị môi trường Sandbox toàn cục cho các tiến trình kiểm thử
$globalSandbox = Join-Path (Get-Location) "tests\sandbox_runner_global"
if (Test-Path $globalSandbox) {
    Remove-Item -Path $globalSandbox -Recurse -Force -ErrorAction SilentlyContinue
}
Copy-Item -Path "data" -Destination $globalSandbox -Recurse -Force
$env:PBL2_TEST_SANDBOX = $globalSandbox
Write-Host "[BẢO MẬT DỮ LIỆU] Đã kích hoạt sandbox môi trường: $globalSandbox" -ForegroundColor Cyan
Write-Host ""

$testList = @(
    @{ Name = "ANTY.md Features & Workflows"; Exe = "tests/test_anty_features.exe" },
    @{ Name = "ANTI.md Critical Security & Persistence Fixes"; Exe = "tests/test_anti_md_fixes.exe" },
    @{ Name = "Full Warranty System (Service & Storage & RBAC)"; Exe = "tests/test_warranty.exe" },
    @{ Name = "Phase A.1 Persistence Hardening & Rollback Transactions"; Exe = "tests/test_persistence_hardening.exe" },
    @{ Name = "Authentication & Real Data Security (PBKDF2/RBAC)"; Exe = "tests/test_auth_real_data.exe" },
    @{ Name = "Admin Reports, Charts & Analytics Formatting"; Exe = "tests/test_admin_reports_charts.exe" },
    @{ Name = "Unified Product Detail View & Pricing Calculations"; Exe = "tests/test_anti_product_detail.exe" },
    @{ Name = "Design System & UI Component Compliance (DESIGN.md)"; Exe = "tests/test_design_md_compliance.exe" },
    @{ Name = "Customer Voucher Wallet & Expiration Logic"; Exe = "tests/test_customer_discounts.exe" },
    @{ Name = "Sales Screen Filtering & Chronological Sorting"; Exe = "tests/test_sales_filter_sort.exe" },
    @{ Name = "Phase A Persistence Core Across Restarts"; Exe = "tests/test_persistence_phase_a.exe" },
    @{ Name = "Phase B3 Voucher, Date Arithmetic & Currency Math"; Exe = "tests/test_phase_b3_voucher.exe" },
    @{ Name = "Phase C Product Image Persistence & File Storage"; Exe = "tests/test_phase_c_image.exe" },
    @{ Name = "Phase D Multi-screen Sync & Real-time Clock Engine"; Exe = "tests/test_phase_d_sync.exe" },
    @{ Name = "Phase E1 UTF-8 Vietnamese Search Consistency"; Exe = "tests/test_phase_e1_search.exe" },
    @{ Name = "Phase E2 & E3 Responsive UI & Anti-overflow Policy"; Exe = "tests/test_phase_e2_ui.exe" },
    @{ Name = "Phase F2 Full End-to-End Business Flow (All 4 Roles)"; Exe = "tests/test_phase_f_e2e.exe" },
    @{ Name = "Phase F1 Windows Native IME & Vietnamese Input Verification"; Exe = "tests/test_ime_final_verification.exe" }
)

$results = @()
$totalPassedSuites = 0
$totalFailedSuites = 0

try {
    foreach ($t in $testList) {
        $name = $t.Name
        $exe = $t.Exe
        Write-Host ">>> ĐANG CHẠY: $name ($exe)..." -ForegroundColor Cyan
        
        $startTime = Get-Date
        $output = & $exe 2>&1
        $exitCode = $LASTEXITCODE
        $duration = ((Get-Date) - $startTime).TotalSeconds

        if ($exitCode -eq 0) {
            Write-Host "    [OK] ĐẠT (Thành công trong $([Math]::Round($duration, 2))s)" -ForegroundColor Green
            $totalPassedSuites++
            $results += [PSCustomObject]@{
                Suite = $name
                Status = "PASSED"
                ExitCode = 0
                Duration = "$([Math]::Round($duration, 2))s"
            }
        } else {
            Write-Host "    [FAIL] LỖI (Exit Code: $exitCode sau $([Math]::Round($duration, 2))s)" -ForegroundColor Red
            $totalFailedSuites++
            $results += [PSCustomObject]@{
                Suite = $name
                Status = "FAILED"
                ExitCode = $exitCode
                Duration = "$([Math]::Round($duration, 2))s"
            }
        }
    }
} finally {
    # 3. Dọn dẹp sandbox runner và gỡ biến môi trường
    $env:PBL2_TEST_SANDBOX = $null
    if (Test-Path $globalSandbox) {
        Remove-Item -Path $globalSandbox -Recurse -Force -ErrorAction SilentlyContinue
    }
}

# 4. Kiểm tra đối chiếu toàn bộ file trong data/ sau khi chạy test
Write-Host ""
Write-Host ">>> ĐANG ĐỐI CHIẾU TÍNH BẤT BIẾN CỦA DỮ LIỆU SẢN XUẤT (data/)..." -ForegroundColor Cyan
$dataMutated = $false
Get-ChildItem -Path "data" -Recurse -File | ForEach-Object {
    $currentHash = (Get-FileHash -Path $_.FullName -Algorithm SHA256).Hash
    if (-not $dataFileHashesBefore.ContainsKey($_.FullName)) {
        Write-Host "    [VI PHẠM] File mới xuất hiện trong data/: $($_.FullName)" -ForegroundColor Red
        $dataMutated = $true
    } elseif ($dataFileHashesBefore[$_.FullName] -ne $currentHash) {
        Write-Host "    [VI PHẠM] File trong data/ bị biến đổi nội dung: $($_.FullName)" -ForegroundColor Red
        $dataMutated = $true
    }
}
foreach ($origFile in $dataFileHashesBefore.Keys) {
    if (-not (Test-Path $origFile)) {
        Write-Host "    [VI PHẠM] File trong data/ bị xóa mất: $origFile" -ForegroundColor Red
        $dataMutated = $true
    }
}

if (-not $dataMutated) {
    Write-Host "    [BẢO TOÀN TUYỆT ĐỐI] 100% tệp trong data/ không hề bị thay đổi hay ghi đè!" -ForegroundColor Green
} else {
    Write-Host "    [CẢNH BÁO BẢO MẬT] Phát hiện tệp trong data/ bị tác động trong quá trình chạy test!" -ForegroundColor Red
}

Write-Host ""
Write-Host "=================================================================="
Write-Host "                      BẢNG TỔNG KẾT KIỂM THỬ                      "
Write-Host "=================================================================="
$results | Format-Table -AutoSize

Write-Host "TỔNG SỐ SUITES THÀNH CÔNG: $totalPassedSuites / $($testList.Count)" -ForegroundColor Green
if ($totalFailedSuites -gt 0) {
    Write-Host "TỔNG SỐ SUITES THẤT BẠI  : $totalFailedSuites" -ForegroundColor Red
} else {
    Write-Host "TẤT CẢ 100% TEST SUITES ĐỀU ĐÃ ĐẠT CHUẨN XUẤT SẮC!" -ForegroundColor Green
}
Write-Host "=================================================================="

if ($dataMutated) {
    exit 2
}
if ($totalFailedSuites -gt 0) {
    exit 1
}
exit 0

