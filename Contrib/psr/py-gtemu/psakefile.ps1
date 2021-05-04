$ErrorActionPreference = 'Stop';

# Path of the root of the repository
$REPO_ROOT_PATH = Join-Path $PSScriptRoot '..\..\..' -Resolve
# Python search path for modules
$VENV_DIR = Join-Path $PSScriptRoot '.venv'
$SCRIPT_DIR = Join-Path $VENV_DIR 'Scripts'

$dirExcludes = '.venv', '.pytest_cache', 'install-test'
$filesToFormat = @(get-childitem -Exclude $dirExcludes -Directory | ForEach-Object { get-childitem -path $_ -Recurse -Include '*.py' })
$filesToFormat += @(get-childitem -Name '*.py')

$PACKAGES = 'cffi', 'ipython', 'pytest', 'hypothesis', 'flake8', 'isort', 'black'

task default -depends isort, Blacken, Flake8, Extension, Test

function executeScript($script, $params) {
    $scriptPath = Join-Path $SCRIPT_DIR -ChildPath $script
    try {
        get-item $scriptPath -ErrorAction Stop > $null
    }
    catch {
        Invoke-Task Packages
    }
    & "$scriptPath" @($params)
    if ($LASTEXITCODE -ne 0 ) {
        throw "$script failed";
    }

}

task isort {
    executeScript 'isort.exe'  $filesToFormat
}

task Blacken {
    executeScript 'black.exe' $filesToFormat
}

task Flake8 {
    executeScript 'flake8.exe' ('gtemu.py', 'gtemu_extension_build.py', 'tests')
}


task Virtualenv {
    try {
        get-item "$VENV_DIR" -ErrorAction Stop > $null
    }
    catch {
        python3 -m venv "$VENV_DIR"
        & "$SCRIPT_DIR\python" @('-m', 'pip', 'install', '--upgrade', 'pip')
    }
}

task Upgrade-Packages {
    executeScript 'pip.exe' (('install', '--upgrade') + $PACKAGES)
}

task Packages -depends VirtualEnv {
    executeScript 'pip.exe' (@(, 'install') + $PACKAGES)
}

task Extension {
    executeScript python.exe .\gtemu_extension_build.py
    if ($LASTEXITCODE -ne 0 ) {
        throw "Extension failed"
    }
}


task RomFiles {
    $testScripts = @(Get-ChildItem 'test-scripts' -Recurse -Include '*.gcl')
    try {
        Push-Location 'roms'
        Remove-Item '*.rom', '*.lst'
    }
    catch {
        New-Item 'roms' -Type Directory | Push-Location
    }
    # We're going to use the roms directory as the working directory when we run the rom.py script
    # So we need to make sure that some expected files are present
    Copy-Item (Join-Path $REPO_ROOT_PATH 'interface.json')
    try {
        Get-Item 'Core' -ErrorAction 'stop' > $null
    }
    catch {
        New-Item -type Directory 'Core' > $null 
    }
    Copy-Item (Join-Path (Join-Path $REPO_ROOT_PATH 'Core') 'v6502.json') 'Core'
    try {
        $testScripts | ForEach-Object {
            executeScript python.exe ((Join-Path (Join-Path $REPO_ROOT_PATH 'Core') 'ROMv4.py'), "Reset=$($_.FullName)")
            if ($LASTEXITCODE -ne 0 ) {
                throw "Failed to generate ROM from $($_.FullName)";
            }
            Rename-Item ROMv4.rom "$($_.Basename).rom"
            Rename-Item ROMv4.lst "$($_.Basename).lst"
        }
    }
    finally {
        Pop-Location
    }
}

task Test -depends RomFiles {
    $oldPythonPath = $env:PYTHONPATH
    $env:PYTHONPATH = Join-Path $REPO_ROOT_PATH 'Core'
    try {
        executeScript 'pytest'
    }
    finally {
        $env:PYTHONPATH = $oldPythonPath
    }
}
