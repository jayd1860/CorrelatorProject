function datetime = copyDateTime(datetime_vec)

datetime = initDateTime();
datetime.date.year   = datetime_vec(1);
datetime.date.month  = datetime_vec(2);
datetime.date.day    = datetime_vec(3);
datetime.time.hour   = datetime_vec(4);
datetime.time.minute = datetime_vec(5);
datetime.time.second = datetime_vec(6);
