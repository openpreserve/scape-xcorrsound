#!/usr/bin/env bash

# Add openplanets Bintray deb repo and udate apt repos
echo "deb http://dl.bintray.com/openplanets/opf-debian /" >> /etc/apt/sources.list
apt-get update

# Install apache 2 and PHP 5 for demo site
apt-get install -y apache2 php5 libapache2-mod-php5

echo "upload_max_filesize = 50M" >> /etc/php5/apache2/php.ini
echo " post_max_size = 192M" >> /etc/php5/apache2/php.ini

# Restart apache and link www root to the vagrant shared dir
# which is the project home directroy. This allows live edits
# on the host machine to be immediately available of the VM.
/etc/init.d/apache2 restart
rm -rf /var/www
ln -fs /vagrant/demosite /var/www




# Install tools for downloading and building xcorrsound
apt-get install -y make cmake ruby-ronn
apt-get install -y libfftw3-dev libboost-all-dev

/vagrant/demosite/install_xcorrsound.sh