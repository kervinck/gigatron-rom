# Copy the up-to-date versions of the upstream files onto a branch, and try to merge that branch

$ErrorActionPreference = 'Stop'


$UPSTREAM_REMOTE = 'upstream'
$UPSTREAM_BRANCH = 'master'
$INTEGRATION_BRANCH = 'py-gtemu-integration'
$INTEGRATION_BRANCH_LAST_COMMIT = Get-Content -Encoding ASCII .\LAST_INTEGRATION_BRANCH_COMMIT

$ROOT = "Contrib/psr/py-gtemu"

$FILES = @(
    # Upstream                      # Our code
    @('interface.json',             'interface.json'),
    @('Core/asm.py',                'asm.py'),
    @('Core/gcl0x.py',              'gcl0x.py'),
    @('Core/ROMv4.py',              'ROMv4.py'),
    @('Docs/gtemu.c',               'gtemu.c')
)


function Invoke-Git() {
    # Run a git command, passing args
    $output = & 'git.exe' $args
    if ($LASTEXITCODE -ne 0) {
        $output | Out-String | Write-Output
        throw "git $($args -join ' ') failed with status code $LASTEXITCODE"
    }
    return $output
}

Invoke-Git fetch $UPSTREAM_REMOTE

try {
    $integrationBranchHead = (Invoke-Git show-ref "--verify" "refs/heads/$INTEGRATION_BRANCH").split()[0]
} catch {
    Invoke-Git branch $INTEGRATION_BRANCH $INTEGRATION_BRANCH_LAST_COMMIT
    $integrationBranchHead = $INTEGRATION_BRANCH_LAST_COMMIT
}

try {
    $upstreamBranchHead = (Invoke-Git show-ref "--verify" "refs/remotes/${UPSTREAM_REMOTE}/${UPSTREAM_BRANCH}").split()[0]
} catch {
    Write-Error "Upstream branch ${UPSTREAM_BRANCH} does not exist in remote ${UPSTREAM_REMOTE}"
    throw;
}

# TODO: Perhaps it would be worth checking that index is clean before throwing it out like this

Invoke-Git read-tree "--empty"

foreach ($pair in $FILES) {
    $src, $dst = $pair
    if ($src -match '/$') {
        $output = Invoke-Git ls-tree "$upstreamBranchHead^{tree}" --full-tree -r -d "$src" 
        assert ($output.length -eq 2) "Not found $src"
        assert ($output[1] -match '\b([a-f0-9]{40})\b') 'No hash in output'
        Invoke-Git read-tree "--prefix=${dst}" $Matches[1]
    } else {
        $output = Invoke-Git ls-tree "$upstreamBranchHead^{tree}" --full-tree -r "$src"
        assert ($null -ne $output) "Not found $src"
        assert ($output -match '\b([a-f0-9]{40})\b') 'No hash in output'
        Invoke-Git update-index --add --cacheinfo "100644,$($Matches[1]),${dst}"
    }
}
# Because I made an error in the past, we also need to add .gitignore, otherwise it will appear as deleted
#Invoke-Git update-index --add --cacheinfo "100644" "0d20b6487c61e7d1bde93acf4a14b7a89083a16d" ".gitignore"



$tree = Invoke-Git write-tree

Invoke-Git read-tree "$upstreamBranchHead^{tree}"
Invoke-Git read-tree "--prefix=${ROOT}" "$tree"

$combinedTree = Invoke-Git write-tree

$commit = Invoke-Git commit-tree $combinedTree -p $integrationBranchHead -p $upstreamBranchHead -m 'Synchronizing upstream files' 

Invoke-Git update-ref "refs/heads/$INTEGRATION_BRANCH" $commit $integrationBranchHead

Set-Content .\LAST_INTEGRATION_BRANCH_COMMIT -Encoding Ascii $commit

Invoke-Git read-tree "HEAD^{tree}"

Invoke-Git merge $INTEGRATION_BRANCH --no-commit
