Files op de USB stick:

**Configuratie van de tool tool**

Instellingen zijn te vinden in de file **/var/config/config.dat.** Na aanpassen van deze file moet de tool (of de Raspberry Pi) opnieuw opgestart worden, omdat deze file door de tool alleen bij het opstarten van het programma uitgelezen wordt.

**Parameters:**

De systeem tijd wordt regelmatig gesynchroniseerd met de tijd die in de testberichten staat. Met de volgende parameter kan je (in minuten) aangeven om de hoeveel tijd dit moet gebeuren. Bereik van deze parameter is 60 – 1440 min:

**UpdateSystemTimeWindow=480**

Met de volgende parameter kan je instellen om de hoeveel tijd (in second) er een sample wordt genomen van de voedingsspanning van de pager Het bereik van deze parameter is 2 – 60 s:

**SampleVoltageWindow=5**

Met de volgende parameter (in seconde) kan ingesteld worden om de hoeveel tijd de gemeten spanning in de logfile gerapporteerd moet worden. In de logfile wordt de laatst gemeten waarde, plus de tot dan toe gemeten minimum en maximum waarde van deze spanning gerapporteerd. Het bereik van deze parameter is 10 – 1200 s:

**ReportVoltageWindow=300**

De volgende parameter (in secondes) is handig als je de tool aan het debuggen bent, bijvoorbeeld via PuTTY of zo. Met deze parameter geef je aan hoe vaak de voedingsspanning van de pager in het terminal window ge-update moet worden. Het bereik van deze parameter is 1 – 1200 s.

**ShowVoltageInTermWindow=5**

De tool logt een foutmelding in de logfile zodra de spanning onder een bepaalde waarde komt. Wordt de spanning hoger of gelijk aan de ingestelde waarde, dan wordt er in de logfile een herstelmelding gezet. Met de volgende parameter kan de betreffen grenswaarde (in Volt) ingesteld worden. Het bereik van deze parameter is 2 – 5 Volt:

**double VoltageMinimumLevel=4.0**

In de directory /logfiles staan de logfiles. Per dag wordt er een logfile aangemaakt met in de filenaam het nummer van de dag. Een logfile wordt maximaal 30 dagen bewaard. Logberichten in deze file waarvan het bericht omgeven is door \*\*\*\*\* en \*\*\*\*\*, zijn meldingen vanuit de tool zelf. Alle andere berichten zijn die welke ontvangen worden via de USB kabel, verbonden met de v.d. Kooij apparatuur.

In de directory /errorlogs staan alle foutmeldingen e.d. zoals die door de tool gegenereerd zijn. Deze meldingen worden in principe ook in de gewone logfile gegeven, maar staan daar tussen de overige berichten. De meldingen in deze file geven daardoor sneller inzicht als er iets fout is gegaan.

De volgende meldingen zijn mogelijk:

- --Meldingen m.b.t. het starten van het programma.
- --N.a.v. het verwijderen en terugplaatsen van de USB stick.
- --Het verbreken of weer terugkomen van de USB verbinding.
- --Als de voedingsspanning van de pager te laag is, of als deze weer groter of gelijk aan de ingestelde waarde is.
- --Het missen van testberichten, als het vorige testbericht langer dan 5,5 minuten geleden ontvangen is wordt dit gerapporteerd in de logfile(s).
