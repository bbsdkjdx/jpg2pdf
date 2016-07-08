import ctypes,sys,threading
dll=ctypes.CDLL(sys.argv[1])
tostrw=lambda x:ctypes.c_wchar_p(x).value
tostra=lambda x:ctypes.c_char_p(x).value
 
def start(fn):
    thd=threading.Thread(target=fn)
    thd.start()
    return thd


