$tests = @(
    "diag_im_query.exe",
    "test_admin_reports_charts.exe",
    "test_anti_product_detail.exe",
    "test_auth_real_data.exe",
    "test_customer_discounts.exe",
    "test_design_md_compliance.exe",
    "test_f1_hwnd.exe",
    "test_f1_windows_input.exe",
    "test_im_unit.exe",
    "test_ime_final_verification.exe",
    "test_ime_modal_lifecycle.exe",
    "test_persistence_hardening.exe",
    "test_persistence_phase_a.exe",
    "test_phase_b1_pos.exe",
    "test_phase_b2_warehouse.exe",
    "test_phase_b3_voucher.exe",
    "test_phase_c_image.exe",
    "test_phase_d_sync.exe",
    "test_phase_e1_search.exe",
    "test_phase_e2_ui.exe",
    "test_warranty.exe"
)

Write-Host "==============================================================="
Write-Host "CHẠY TOÀN BỘ BỘ TEST VÀ SO SÁNH VỚI BASELINE"
Write-Host "==============================================================="

$results = @()

foreach ($t in $tests) {
    $path = "tests/$t"
    if (-not (Test-Path $path)) {
        $results += [PSCustomObject]@{ Test = $t; Status = "NOT FOUND"; ExitCode = -1 }
        continue
    }

    Write-Host "Đang chạy $t..."
    $proc = Start-Process -FilePath $path -NoNewWindow -PassThru -Wait
    $code = $proc.ExitCode

    $status = if ($code -eq 0) { "PASS" } else { "FAIL" }
    $results += [PSCustomObject]@{ Test = $t; Status = $status; ExitCode = $code }
}

Write-Host ""
Write-Host "==============================================================="
Write-Host "KẾT QUẢ TỔNG HỢP KIỂM THỬ"
Write-Host "==============================================================="
$results | Format-Table -AutoSize

