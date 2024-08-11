$OriginalDirectory = Get-Location
try {
  Set-Location -Path $PSScriptRoot
  wix build -ext WixToolset.UI.wixext -arch x86 .\TrinoODBC_x86.wxs
}
finally {
  Set-Location -Path $OriginalDirectory
}