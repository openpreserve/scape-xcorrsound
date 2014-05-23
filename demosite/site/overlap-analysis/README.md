# Demonstration site for xcorrsound

Requirements
The sample wave files need to be present on the server, this should be taken care of using vagrant
that will download the sample files during setup.

Configuration

PHP
The following parameters in php.ini should changed to allow upload of larger files.

  upload_max_filesize = 50M
  post_max_size = 192M
