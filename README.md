# Python based Telegram VOIP calls

# Warning: this is not yet ready to use

## Build with Docker
```
docker build --rm -t nlhomme/pytgvoip .
```

You need to register an app on Telegram's website, retrieve the API id and hash.
Then you can call someone.
```
python3 tgcall.py api_id api_hash phone user_id dbkey
```
