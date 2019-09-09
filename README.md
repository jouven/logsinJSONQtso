# logsinJSONQtso
Log library, logs can be saved, file-wise, in JSON format or in a "custom" csv format (log entry = 1 text line)

Also uses barely no memory when the log entry text are duplicates of previous logs, which can happen pretty often

Compilation
-----------
Requires:

Qt library 

https://github.com/jouven/essentialQtso

https://github.com/jouven/criptoQtso

Run (in logsinJSONQtso source directory or pointing to it):

    qmake

and then:

    make
