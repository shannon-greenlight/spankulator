rem - "C:\Program Files\KiCad\bin\xsltproc" -o "Spankulator.csv" "C:\Program Files\KiCad\bin\scripting\plugins\bom2grouped_csv.xsl" "C:\Users\shann\Dropbox\moog\spankulator\Spankulator-Hardware\Spankulator.xml"

"C:\Program Files\KiCad\bin\python" "C:\Users\shann\Dropbox\KiCAD\BOM/bom_lcsc.py" "Spankulator.xml" "bom\LCSC\spankulator_bom.csv"

"C:\Program Files\KiCad\bin\python" "C:\Users\shann\Dropbox\KiCAD\BOM/bom_digikey_split.py" "Spankulator.xml" "bom\Digi-Key\spankulator_split.csv"

"C:\Program Files\KiCad\bin\python" "C:\Users\shann\Dropbox\KiCAD\BOM/bom_digikey_full.py" "Spankulator.xml" "bom\Digi-Key\spankulator_full.csv"

pause
