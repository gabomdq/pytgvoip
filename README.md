# Python based Telegram VOIP calls

## The easy way: Build with Docker
```
docker build -t pytgvoip .
```

## Or, build manually

### Build and Install libtgvoip
```
git clone https://github.com/gabomdq/libtgvoip.git
cd libtgvoip
./configure
make
make install
```

### Build tdlib
```
git clone https://github.com/tdlib/td.git
cd td
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### Build and Install tdlib Python wrapper
```
git clone https://github.com/gabomdq/python-telegram.git
cd python-telegram
```
(Before installing!) Copy libtdjson.so from the previous step to python-telegram/telegram/lib/linux
```
python3 setup.py install --user
```

### Install this extension
```
python3 setup.py install --user
```

You need to register an app on Telegram's website, retrieve the API id and hash.
Then you can call someone.
```
python3 tgcall.py api_id api_hash phone user_id dbkey
```
