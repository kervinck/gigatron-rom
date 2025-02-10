How to use this code.


* On a web server: copy these files into a directory your web site.
  Add a file `gigatron.rom` with the ROM you want to offer by default and a file `sd.vhd` with a disk image.  When using the Apache web server, it is possible to make things much more efficient by providing instead gzipped files `gigatron.rom.gz` and `sd.vhd.gz` and using a `.htaccess` file with the following incantations (you need the Apache modules `mod_headers` and `mod_rewrite`.)
  ```
      <IfModule mod_headers.c>
          Header merge Cache-Control private,max-age=86400,must-revalidate
      </IfModule>
      <IfModule mod_rewrite.c>
          RewriteCond "%{HTTP:Accept-encoding}" "gzip"
          RewriteCond "%{REQUEST_FILENAME}\.gz" -s
          RewriteRule "^(.*)\.(rom|vhd)"         "$1\.$2\.gz" [QSA]
          # Serve correct content types, and prevent mod_deflate double gzip.
          RewriteRule "\.(rom|vhd)\.gz$" "-" [T=application/octet-stream,E=no-gzip:1]
          # Serve correct encoding type.
          <FilesMatch "\.(rom|vhd)\.gz$">
            Header append Content-Encoding gzip
            Header append Vary Accept-Encoding
          </FilesMatch>
      </IfModule>
  ```

* Locally: from the `gigatron-rom` directory, type 
  ``` 
  $ make runjs ROM=xxx.rom
  ```
  where `xxx.rom` is the ROM you want to execute. This uses the python-based web server found in the parent directory. It requires python>=3.7.  It also will serve gzipped rom and vhd files if they're available.
