import cantools

db = cantools.database.load_file('dbc/system.dbc')

msg = db.get_message_by_name('SENSOR_DATA')
data = msg.encode({'temperature': 23.5, 'alive_counter':3, 'checksum':42})
print("Encoded bytes:", data.hex())

decoded = msg.decode(data)
print("Decoded:", decoded)


