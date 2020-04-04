$ErrorActionPreference = 'Stop';

$dirExcludes = '.venv', '.pytest_cache'
$fileExcludes = @()
$filesToFormat = get-childitem -Exclude $dirExcludes -Directory | % { get-childitem -path $_ -Recurse -Include '*.py' }
$filesToFormat += get-item 'gtemu_extension_build.py'

task default -depends isort, Blacken, Flake8, Extension, Test

task isort {
    & '.\.venv\Scripts\isort.exe' $filesToFormat
    if ($LASTEXITCODE -ne 0 ) {
        throw "isort failed";
    }
}

task Blacken {
    & '.\.venv\Scripts\black.exe' $filesToFormat
    if ($LASTEXITCODE -ne 0 ) {
        throw "Black failed";
    }
}

task Flake8 {
    & '.\.venv\Scripts\flake8.exe' 'src' 'tests'
    if ($LASTEXITCODE -ne 0 ) {
        throw "Flake8 failed";
    }
}

task Test {
    .\.venv\Scripts\pytest
    if ($LASTEXITCODE -ne 0 ) {
        throw "Test failure";
    }
}

task Virtualenv {
    try {
        get-item '.venv' -ErrorAction Stop > $null
    }
    catch {
        py -3.6 -m venv '.\.venv'
    }
}

task Upgrade-Packages -depends Virtualenv {
    .\.venv\Scripts\python -m pip install --upgrade pip cffi ipython pytest hypothesis flake8
}

task Packages -depends Virtualenv {
    .\.venv\Scripts\pip install cffi ipython pytest hypothesis flake8 isort black
    if ($LASTEXITCODE -ne 0 ) {
        throw "Packages failed"
    }
}

task Extension -depends Packages {
    .\.venv\Scripts\python.exe .\gtemu_extension_build.py
    if ($LASTEXITCODE -ne 0 ) {
        throw "Extension failed"
    }
}


task ROM {
    .\.venv\Scripts\python.exe .\dev.py Main=MainMenu\MainMenu.gcl Boot=CardBoot.gcl Reset=Reset.gcl
    if ($LASTEXITCODE -ne 0 ) {
        throw "Rom failed";
    }
}
