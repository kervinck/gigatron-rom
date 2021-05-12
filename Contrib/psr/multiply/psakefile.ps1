$ErrorActionPreference = 'Stop';

# Path of the root of the repository
$REPO_ROOT_PATH = Join-Path $PSScriptRoot '..\..\..' -Resolve
# Python search path for modules
$VENV_DIR = Join-Path $PSScriptRoot '.venv'
$SCRIPT_DIR = Join-Path $VENV_DIR 'Scripts'
$SRC_DIR = Join-Path $PSScriptRoot 'src'
$OUT_DIR = Join-Path $PSScriptRoot 'out'
$TEST_DIR = Join-Path $PSScriptRoot 'test'

$dirExcludes = '.venv', '.pytest_cache'

function findFilesToFormat() {
    $filesToFormat = @(Get-ChildItem -Exclude $dirExcludes -Directory | ForEach-Object { Get-ChildItem -path $_ -Recurse -Filter '*.py' })
    $filesToFormat += @(Get-ChildItem -Filter '*.py')
    return @($filesToFormat | ForEach-Object { $_.FullName })
}

$PACKAGES = 'ipython', 'pytest', 'hypothesis', 'flake8', 'isort', 'black'

task default -depends isort, Blacken, Flake8, Test, Listing

function executeScript($script, $params) {
    $scriptPath = Join-Path $SCRIPT_DIR -ChildPath $script
    try {
        get-item $scriptPath -ErrorAction Stop > $null
    }
    catch {
        Invoke-Task Packages
    }
    exec { & "$scriptPath" @($params) } -errorMessage "Error when executing ${script}"
}

task isort {
    executeScript 'isort.exe' (findFilesToFormat)
}

task Blacken {
    executeScript 'black.exe' (findFilesToFormat)
}

task Flake8 {
    $filesToFormat = findFilesToFormat
    if ($filesToFormat.Count -gt 0) {
        executeScript 'flake8.exe' $filesToFormat
    }
}

task Virtualenv {
    try {
        get-item "$VENV_DIR" -ErrorAction Stop > $null
    }
    catch {
        exec { python3 -m venv "$VENV_DIR" } -workingDirectory $PSScriptRoot
        executeScript 'python.exe' @('-m', 'pip', 'install', '--upgrade', 'pip', 'setuptools', 'wheel')
        # We always want core in the repo root to be on the search path
        $pthFile = (Join-Path (Join-Path (Join-Path $VENV_DIR 'Lib') 'site-packages') 'gigatron-core.pth')
        New-Item $pthFile > $null
        "$(join-path $REPO_ROOT_PATH 'Core')" | Set-Content -Encoding 'ASCII' $pthFile
    }
}

task Upgrade-Packages {
    executeScript 'pip.exe' (('install', '--upgrade') + $PACKAGES)
    Push-Location (Join-Path $PSScriptRoot '..\..\psr\py-gtemu')
    try {
        Remove-Item 'dist' -Recurse -ErrorAction SilentlyContinue
        executeScript 'python.exe' @('setup.py', 'bdist_wheel')
        $wheel = Get-ChildItem 'dist' '*.whl'
        executeScript 'pip.exe' @('install', '--force-reinstall', $wheel.FullName)
    }
    finally {
        Pop-Location
    }
}

task Packages -depends VirtualEnv {
    # In case we're not already here
    Push-Location $PSScriptRoot
    try {
        Push-Location '..\..\psr\py-gtemu'
        try {
            $wheel = Get-ChildItem 'dist' '*.whl' -ErrorAction SilentlyContinue
            if (!$wheel) {
                executeScript 'python.exe' @('setup.py', 'bdist_wheel')
            }
            executeScript 'pip.exe' @('install', $wheel.FullName)
        }
        finally {
            Pop-Location
        }
    }
    finally { 
        Pop-Location 
    }
    Pop-Location
    executeScript 'pip.exe' (@(, 'install') + $PACKAGES)
}

task Test {
    if (Test-Path $TEST_DIR) {
        executeScript 'pytest.exe'
    }
}

task Listing {
    try { 
        $output = Get-Item $OUT_DIR -ErrorAction Stop 
    }
    catch {
        $output = New-Item $OUT_DIR -Type Directory 
    }
    Push-Location $output
    try {
        Get-ChildItem $SRC_DIR '*.asm.py' -ErrorAction SilentlyContinue | ForEach-Object {
            executeScript 'python.exe' @(, $_.FullName)
        }
    }
    finally {
        Pop-Location
    }
}

task Clean {
    get-item $OUT_DIR -ErrorAction SilentlyContinue | Remove-Item -Recurse
    exec { git clean '-fXd' }
}

task PreCommit {
    # Save any unknown / unstaged files
    exec { git stash push '-kum' "Unstaged changes, for precommit testing" }
    Push-Location $PSScriptRoot
    try {
        exec { git clean '-fdxe' '.venv' '-e' '.pytest_cache' }
        Invoke-Task default -ErrorAction Stop
        # Check for any unstaged changes resulting
        # we should be committing everything - including .lst files
        exec { git 'update-index' '--refresh' '-q' }
        $gitStatusOutput = @(exec { git 'status' '--porcelain=v1' '-uall' })
        $modifiedFiles = @($gitStatusOutput -match '^.[^ ]')
        Assert ($modifiedFiles.Count -eq 0) -failureMessage "Unstaged files were modified"
        exec { git stash pop }
    }
    finally { 
        Pop-Location 
    } 
}
