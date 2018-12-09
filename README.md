# DHT22  


### Build

Follow [this](https://github.com/easyqiot/esp-env) instruction 
to setup your environment.


```bash
source esp-env/nonos/activate.sh

cd esp-env/nonos/sdk/
git clone git@github.com:easyqiot/dht22.git
cd dht22 

./gen.sh
make flash
```

OR

```bash
make fota
```
