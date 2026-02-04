To install the ftd2xx library, consult
https://ftdichip.com/drivers/d2xx-drivers/. Pay particular attention to the
ReadMe link, which contains install instructions.

Or if you're lazy, just do this:

```bash
cd ~
wget https://ftdichip.com/wp-content/uploads/2022/07/libftd2xx-arm-v8-1.4.27.tgz
tar xfvz libftd2xx-arm-v8-1.4.27.tgz
cd release
cd build
sudo su # AS ROOT
cp libftd2xx.* /usr/local/lib
chmod 0755 /usr/local/lib/libftd2xx.so.1.4.27 
ln -sf /usr/local/lib/libftd2xx.so.1.4.27 /usr/local/lib/libftd2xx.so
exit    # NO LONGER ROOT
cd ~
rm libftd2xx-arm-v8-1.4.27.tgz
rm -r release
sudo ldconfig
cd ~/load-cells-futek/

make # Unrelated to ftd2xx library
```

Make sure to run the following after following the steps to install the shared
library:

```bash
sudo ldconfig
```
The Linux kernel automatically loads ftdi_sio, which locks the hardware and prevents the D2XX library (used by ./main) from accessing the sensors. Run the following to give permanent access to Pi: 
```
echo "blacklist ftdi_sio" | sudo tee /etc/modprobe.d/ftdi-blacklist.conf
echo "blacklist usbserial" | sudo tee -a /etc/modprobe.d/ftdi-blacklist.conf
sudo depmod -a
```
To undo, run:
```
sudo rm /etc/modprobe.d/ftdi-blacklist.conf
sudo depmod -a
sudo reboot
```
