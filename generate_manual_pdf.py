# -*- coding: utf-8 -*-
import os
import sys
from reportlab.lib import colors
from reportlab.lib.pagesizes import A4
from reportlab.lib.units import cm
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.platypus import (
    SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, PageBreak, KeepTogether, HRFlowable
)
from reportlab.pdfgen import canvas

class NumberedCanvas(canvas.Canvas):
    def __init__(self, *args, **kwargs):
        super(NumberedCanvas, self).__init__(*args, **kwargs)
        self._saved_page_states = []

    def showPage(self):
        self._saved_page_states.append(dict(self.__dict__))
        self._startPage()

    def save(self):
        num_pages = len(self._saved_page_states)
        for state in self._saved_page_states:
            self.__dict__.update(state)
            self.draw_page_decorations(num_pages)
            super(NumberedCanvas, self).showPage()
        super(NumberedCanvas, self).save()

    def draw_page_decorations(self, page_count):
        self.saveState()
        self.setFont("Helvetica", 8)
        self.setFillColor(colors.HexColor("#64748b"))
        
        # Header (dalla pagina 2 in poi)
        if self._pageNumber > 1:
            self.drawString(1.5 * cm, A4[1] - 1.2 * cm, "Termostato Touch Smart AZ-Touch 2.8\" &mdash; Manuale d'Uso")
            self.setStrokeColor(colors.HexColor("#cbd5e1"))
            self.setLineWidth(0.5)
            self.line(1.5 * cm, A4[1] - 1.35 * cm, A4[0] - 1.5 * cm, A4[1] - 1.35 * cm)
            
        # Footer
        page_text = f"Pagina {self._pageNumber} di {page_count}"
        self.drawRightString(A4[0] - 1.5 * cm, 1.0 * cm, page_text)
        self.drawString(1.5 * cm, 1.0 * cm, "Sistema Multi-Piano | Home Assistant MQTT Auto-Discovery")
        self.setStrokeColor(colors.HexColor("#cbd5e1"))
        self.setLineWidth(0.5)
        self.line(1.5 * cm, 1.35 * cm, A4[0] - 1.5 * cm, 1.35 * cm)
        self.restoreState()

def build_pdf(filename="Manuale_Istruzioni_Termostato_Smart.pdf"):
    doc = SimpleDocTemplate(
        filename,
        pagesize=A4,
        leftMargin=1.5 * cm,
        rightMargin=1.5 * cm,
        topMargin=1.8 * cm,
        bottomMargin=1.8 * cm
    )

    styles = getSampleStyleSheet()
    
    # Custom Palette
    c_primary = colors.HexColor("#0f172a")     # Slate 900
    c_secondary = colors.HexColor("#0369a1")   # Sky 700
    c_accent = colors.HexColor("#0284c7")      # Sky 600
    c_dark = colors.HexColor("#1e293b")        # Slate 800
    c_light_bg = colors.HexColor("#f8fafc")    # Slate 50
    c_card_bg = colors.HexColor("#f1f5f9")     # Slate 100
    c_border = colors.HexColor("#e2e8f0")
    
    # Custom Styles
    style_title = ParagraphStyle(
        'DocTitle',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=24,
        leading=28,
        textColor=colors.HexColor("#0369a1"),
        alignment=1, # Center
        spaceAfter=6
    )
    
    style_subtitle = ParagraphStyle(
        'DocSubTitle',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=13,
        leading=17,
        textColor=colors.HexColor("#475569"),
        alignment=1,
        spaceAfter=15
    )
    
    style_badge = ParagraphStyle(
        'Badge',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=9,
        leading=12,
        textColor=colors.HexColor("#0284c7"),
        alignment=1,
        spaceAfter=20
    )

    style_h1 = ParagraphStyle(
        'Heading1_Custom',
        parent=styles['Heading1'],
        fontName='Helvetica-Bold',
        fontSize=15,
        leading=19,
        textColor=colors.HexColor("#0f172a"),
        spaceBefore=14,
        spaceAfter=8,
        keepWithNext=True
    )
    
    style_h2 = ParagraphStyle(
        'Heading2_Custom',
        parent=styles['Heading2'],
        fontName='Helvetica-Bold',
        fontSize=12,
        leading=16,
        textColor=colors.HexColor("#0369a1"),
        spaceBefore=10,
        spaceAfter=4,
        keepWithNext=True
    )

    style_body = ParagraphStyle(
        'Body_Custom',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=9.5,
        leading=14,
        textColor=colors.HexColor("#334155"),
        spaceAfter=6
    )
    
    style_body_bold = ParagraphStyle(
        'BodyBold_Custom',
        parent=style_body,
        fontName='Helvetica-Bold'
    )

    style_box_text = ParagraphStyle(
        'BoxText',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=9,
        leading=13.5,
        textColor=colors.HexColor("#1e293b")
    )
    
    style_table_cell = ParagraphStyle(
        'TableCell',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=8.5,
        leading=12,
        textColor=colors.HexColor("#1e293b")
    )

    style_table_header = ParagraphStyle(
        'TableHeader',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=9,
        leading=12,
        textColor=colors.white
    )

    story = []

    def make_box(content_para, bg_color="#f1f5f9", border_color="#0284c7"):
        t = Table([[content_para]], colWidths=[A4[0] - 3.0 * cm])
        t.setStyle(TableStyle([
            ('BACKGROUND', (0,0), (-1,-1), colors.HexColor(bg_color)),
            ('BOX', (0,0), (-1,-1), 1.5, colors.HexColor(border_color)),
            ('TOPPADDING', (0,0), (-1,-1), 7),
            ('BOTTOMPADDING', (0,0), (-1,-1), 7),
            ('LEFTPADDING', (0,0), (-1,-1), 10),
            ('RIGHTPADDING', (0,0), (-1,-1), 10),
        ]))
        return t

    # --- COPERTINA / INTESTAZIONE ---
    story.append(Spacer(1, 10))
    story.append(Paragraph("TERMOSTATO SMART MULTI-PIANO AZ-TOUCH 2.8\"", style_title))
    story.append(Paragraph("Guida Completa all'Uso e alla Configurazione &mdash; Versione per Tutti gli Utenti", style_subtitle))
    story.append(Paragraph("<b>Compatibile con:</b> Display Touch a Colori 2.8\" | Sensore Ambientale BME280 | Home Assistant & MQTT", style_badge))
    story.append(HRFlowable(width="100%", thickness=1.5, color=colors.HexColor("#0284c7"), spaceAfter=15))

    intro_p = Paragraph(
        "<b>Gentile Utente,</b> benvenuto nella guida all'uso del tuo nuovo Termostato Smart. "
        "Questo dispositivo è progettato per offrirti il massimo comfort domestico e un sensibile risparmio sui consumi di riscaldamento, "
        "grazie alla gestione autonoma avanzata (cronotermostato, rilevamento finestra aperta, protezione antigelo e monitoraggio continuo) "
        "e alla piena sincronizzazione con <b>Home Assistant</b> e smartphone.<br/>"
        "Non è richiesta alcuna competenza di programmazione: seguendo questa guida passo-passo potrai configurare e controllare tutto in pochi minuti.",
        style_box_text
    )
    story.append(make_box(intro_p, "#f8fafc", "#0284c7"))
    story.append(Spacer(1, 12))

    # --- SEZIONE 1: PRIMA ACCENSIONE E WIFI ---
    story.append(Paragraph("1. Prima Accensione e Connessione al Wi-Fi di Casa", style_h1))
    story.append(Paragraph(
        "Al primo avvio, o qualora il termostato non trovi la rete Wi-Fi salvata, si attiva automaticamente la modalità di configurazione facile:",
        style_body
    ))
    
    wifi_steps = [
        "<b>Passo 1 &mdash; Collegati con il cellulare o PC:</b> apri le impostazioni Wi-Fi del tuo smartphone e cerca la rete aperta denominata <b>Termostato_AP</b> (non richiede password iniziale).",
        "<b>Passo 2 &mdash; Pagina di Benvenuto:</b> una volta connesso, si aprirà automaticamente la schermata di configurazione (in alternativa, apri Chrome o Safari e digita l'indirizzo <b>192.168.4.1</b>).",
        "<b>Passo 3 &mdash; Selezione Rete:</b> clicca sul pulsante <b>'Scansiona Reti Wi-Fi'</b>, tocca il nome della tua rete Wi-Fi domestica, inserisci la relativa password e premi <b>'Salva e Connetti al Wi-Fi'</b>.",
        "<b>Passo 4 &mdash; Verifica sul Display:</b> dopo pochi secondi, sul display del termostato (in basso a destra) scomparirà la scritta <i>WiFi init</i> e apparirà l'indirizzo IP locale (es. <code>192.168.1.150</code>). La data e l'ora si sincronizzeranno istantaneamente via internet."
    ]
    for step in wifi_steps:
        story.append(Paragraph(f"&bull; {step}", style_body))
    story.append(Spacer(1, 10))

    # --- SEZIONE 2: SCHERMO TOUCH E COMANDI MANUALI ---
    story.append(Paragraph("2. Uso Quotidiano dal Display Touch Screen", style_h1))
    story.append(Paragraph(
        "Il display LCD a colori mostra a colpo d'occhio tutti i parametri del tuo ambiente e permette un controllo rapido con un semplice tocco:",
        style_body
    ))

    t_data = [
        [Paragraph("Elemento Display", style_table_header), Paragraph("Descrizione &amp; Funzionamento", style_table_header)],
        [
            Paragraph("<b>Pulsanti [+] e [-]</b>", style_table_cell),
            Paragraph("Aumentano o diminuiscono la temperatura desiderata (setpoint) a scatti di 0.1 °C. Ad ogni tocco il buzzer emette un delicato 'tick' acustico.", style_table_cell)
        ],
        [
            Paragraph("<b>Pulsante [MODE]</b>", style_table_cell),
            Paragraph("Alterna a rotazione la modalità:<br/>&bull; <b>Caldo (Rosso):</b> per l'inverno (attiva la caldaia se T &lt; Setpoint).<br/>&bull; <b>Freddo (Blu):</b> per l'estate / climatizzazione.<br/>&bull; <b>Spento (Grigio):</b> relè a riposo.", style_table_cell)
        ],
        [
            Paragraph("<b>Riquadro INDOOR</b>", style_table_cell),
            Paragraph("Mostra in grande la <b>temperatura attuale (°C)</b> e l'<b>umidità relativa (%)</b> misurate dal sensore ad altissima precisione BME280.", style_table_cell)
        ],
        [
            Paragraph("<b>Meteo &amp; Barometro</b>", style_table_cell),
            Paragraph("Sotto l'umidità compare la pressione atmosferica (es. <i>1013hPa Sole | Ottimo</i>), la tendenza meteo delle prossime ore e l'indice di comfort.", style_table_cell)
        ],
        [
            Paragraph("<b>Grafico a Linee</b>", style_table_cell),
            Paragraph("Nella metà inferiore dello schermo, un grafico interattivo disegna in tempo reale la curva dell'andamento termico delle ultime ore.", style_table_cell)
        ],
        [
            Paragraph("<b>Barra Segnale Wi-Fi</b>", style_table_cell),
            Paragraph("In alto a destra (accanto all'orologio), 4 barrette dinamiche colorate indicano l'intensità del segnale Wi-Fi:<br/>&bull; <b>4 barre verdi:</b> Segnale eccellente (&ge; -60 dBm).<br/>&bull; <b>3 barre verde chiaro:</b> Segnale buono (&ge; -70 dBm).<br/>&bull; <b>2 barre gialle:</b> Segnale sufficiente (&ge; -80 dBm).<br/>&bull; <b>1 barra rossa:</b> Segnale debole (&lt; -80 dBm).<br/><i>(Colore ciano continuo in modalità configurazione AP).</i>", style_table_cell)
        ],
        [
            Paragraph("<b>Risveglio Display</b>", style_table_cell),
            Paragraph("Dopo 2 minuti di inattività, il display attenua la luminosità per risparmiare energia e non disturbare al buio. Basta toccare un punto qualsiasi dello schermo per riattivarlo.", style_table_cell)
        ]
    ]
    t_display = Table(t_data, colWidths=[4.2 * cm, A4[0] - 3.0 * cm - 4.2 * cm])
    t_display.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), colors.HexColor("#0f172a")),
        ('GRID', (0,0), (-1,-1), 0.5, colors.HexColor("#cbd5e1")),
        ('ROWBACKGROUNDS', (0,1), (-1,-1), [colors.white, colors.HexColor("#f8fafc")]),
        ('VALIGN', (0,0), (-1,-1), 'TOP'),
        ('TOPPADDING', (0,0), (-1,-1), 5),
        ('BOTTOMPADDING', (0,0), (-1,-1), 5),
    ]))
    story.append(t_display)
    story.append(Spacer(1, 14))

    # --- SEZIONE 3: PAGINA WEB DEL TERMOSTATO ---
    story.append(Paragraph("3. Controllo da Pagina Web (Senza Installare Nessuna App)", style_h1))
    story.append(Paragraph(
        "Basta digitare l'indirizzo IP del termostato (mostrato in basso sullo schermo, es. <code>http://192.168.1.150</code>) "
        "nella barra degli indirizzi di qualsiasi browser (da cellulare, tablet o PC connesso al Wi-Fi di casa) per visualizzare il pannello di controllo completo suddiviso in comode schede:",
        style_body
    ))

    web_features = [
        "<b>Stato Termostato &amp; Tasti Rapidi:</b> vedi temperatura, umidità, setpoint e stato del relè in tempo reale. Pulsanti rapidi per variare la temperatura (-0.5, -0.1, +0.1, +0.5 °C) o impostare istantaneamente il modo <b>Eco (18 °C)</b> o <b>Boost (23 °C)</b>.",
        "<b>Potenza Segnale Wi-Fi:</b> barra grafica colorata percentuale (0&ndash;100%) con misurazione RSSI in dBm e giudizio qualitativo istantaneo (<i>Eccellente</i>, <i>Buono</i>, <i>Debole</i>).",
        "<b>Regolazione Volume Buzzer:</b> quattro pulsanti dedicati per scegliere il livello del suono al tocco: <i>Muto</i>, <i>Basso (Tick leggero)</i>, <i>Medio</i> o <i>Alto</i>.",
        "<b>Meteo Barometrico &amp; Comfort Ambientale:</b> calcolo del <b>Punto di Rugiada</b> (utile per prevenire muffe e condensa sui muri), della <b>Temperatura Percepita Humidex</b> e previsione del tempo basata sulla variazione della pressione barometrica a 3 ore.",
        "<b>Diagnostica &amp; Scanner I2C:</b> permette di testare istantaneamente il collegamento elettrico dei sensori con un click, visualizzando i voltaggi e il chip identificato."
    ]
    for wf in web_features:
        story.append(Paragraph(f"&bull; {wf}", style_body))
    story.append(Spacer(1, 10))

    story.append(PageBreak())

    # --- SEZIONE 4: NUOVE FUNZIONALITÀ INTELLIGENTI ---
    story.append(Paragraph("4. Nuove Funzionalità Intelligenti di Comfort e Risparmio", style_h1))
    
    # 4.1 Cronotermostato
    story.append(Paragraph("4.1 Cronotermostato Settimanale (Programmazione Oraria)", style_h2))
    story.append(Paragraph(
        "Il termostato gestisce autonomamente <b>8 fasce orarie indipendenti</b>: "
        "4 fasce per i giorni feriali (<b>Lunedì &ndash; Venerdì</b>) e 4 fasce per il weekend (<b>Sabato &ndash; Domenica</b>). "
        "Ciascuna fascia oraria prevede l'orario di inizio e la temperatura desiderata:<br/>"
        "&bull; <b>1. Mattina:</b> risveglio al caldo (es. 06:30 &rarr; 20.5 °C).<br/>"
        "&bull; <b>2. Giorno:</b> risparmio quando si è al lavoro o a scuola (es. 08:30 &rarr; 18.5 °C).<br/>"
        "&bull; <b>3. Sera:</b> massimo comfort per il rientro a casa (es. 17:00 &rarr; 21.0 °C).<br/>"
        "&bull; <b>4. Notte:</b> temperatura ideale per il sonno (es. 22:30 &rarr; 18.0 °C).<br/>"
        "Nella pagina web è presente il comodo tasto <b>'Copia Feriali su Weekend'</b> per duplicare la programmazione in un click. "
        "Quando il cronotermostato è attivo, sul display TFT viene mostrata la fascia in corso (es. <code>CRONO SERA</code>).",
        style_body
    ))
    story.append(Spacer(1, 6))

    # 4.2 Protezione Antigelo
    story.append(Paragraph("4.2 Protezione Antigelo Automatica (Sempre Attiva per Sicurezza)", style_h2))
    story.append(Paragraph(
        "Se parti per le vacanze o decidi di impostare il termostato su <b>Spento</b>, non rischi il congelamento e la rottura dei tubi dell'impianto idraulico. "
        "Qualora la temperatura della stanza scenda a <b>5.0 °C o meno</b>, il termostato avvia immediatamente la caldaia in modalità di emergenza fino a riportare l'ambiente ad almeno 6.0 °C, "
        "mostrando l'avviso <code>ANTIGELO 5C</code> a schermo.",
        style_body
    ))
    story.append(Spacer(1, 6))

    # 4.3 Finestra Aperta
    story.append(Paragraph("4.3 Rilevamento Intelligente 'Finestra Aperta'", style_h2))
    story.append(Paragraph(
        "Mentre la caldaia è in funzione, se apri le finestre per cambiare aria la temperatura crolla rapidamente. "
        "Il termostato rileva questo abbassamento anomalo (<b>calo &ge; 1.5 °C in pochi minuti</b>) e <b>mette automaticamente in pausa il riscaldamento per 20 minuti</b>, "
        "mostrando sul display <code>FINESTRA AP</code> ed evitando di bruciare gas inutilmente. Passati 20 minuti, il riscaldamento riparte da solo.",
        style_body
    ))
    story.append(Spacer(1, 6))

    # 4.4 Ore di Accensione
    story.append(Paragraph("4.4 Contatore Ore di Accensione Caldaia &amp; Storico Consumi", style_h2))
    story.append(Paragraph(
        "Nella card <b>'Sicurezza &amp; Consumi Caldaia'</b> della pagina web (e nei sensori di Home Assistant) puoi verificare con precisione:<br/>"
        "&bull; <b>Accensione Oggi:</b> tempo esatto di funzionamento odierno del riscaldamento (es. <i>2h 35m</i>).<br/>"
        "&bull; <b>Accensione 7 Giorni:</b> somma complessiva delle ore bruciate nell'ultima settimana.<br/>"
        "&bull; <b>Storico Giorno per Giorno:</b> una tabella con i consumi dei singoli giorni precedenti (da <i>-1gg</i> a <i>-7gg</i>), aggiornata automaticamente a mezzanotte.",
        style_body
    ))
    story.append(Spacer(1, 14))

    # --- SEZIONE 5: INTEGRAZIONE CON HOME ASSISTANT ---
    story.append(Paragraph("5. Integrazione con Home Assistant (Configurazione per 3 Piani)", style_h1))
    story.append(Paragraph(
        "Grazie al protocollo <b>MQTT Auto-Discovery</b>, non dovrai scrivere nessuna riga di codice in Home Assistant: "
        "tutte le entità, i sensori, gli interruttori e la scheda clima compaiono da soli!",
        style_body
    ))

    ha_box_p = Paragraph(
        "<b>Regola d'Oro per i 3 Termostati di Casa (Seminterrato, Piano Terra, Primo Piano):</b><br/>"
        "Ogni termostato riconosce automaticamente a quale piano appartiene semplicemente dal <b>Topic Base</b> impostato nella card MQTT della pagina web:<br/>"
        "&bull; Per il Seminterrato: <code>home/clima/seminterrato/</code> &rarr; Crea il clima: <b>Clima Seminterrato</b><br/>"
        "&bull; Per il Piano Terra: <code>home/clima/piano_terra/</code> &rarr; Crea il clima: <b>Clima Piano Terra</b><br/>"
        "&bull; Per il Primo Piano: <code>home/clima/primo_piano/</code> &rarr; Crea il clima: <b>Clima Primo Piano</b><br/>"
        "Puoi selezionare il tuo piano con un solo click grazie ai 3 pulsanti rapidi presenti nella pagina web del termostato!",
        style_box_text
    )
    story.append(make_box(ha_box_p, "#f8fafc", "#0284c7"))
    story.append(Spacer(1, 10))

    story.append(Paragraph("<b>Elenco delle entità create automaticamente in Home Assistant per ogni piano:</b>", style_body_bold))
    
    ha_entities = [
        [Paragraph("Entità Home Assistant", style_table_header), Paragraph("Tipo", style_table_header), Paragraph("Funzione &amp; Utilizzo", style_table_header)],
        [
            Paragraph("<code>climate.termostato_&lt;piano&gt;</code>", style_table_cell),
            Paragraph("Termostato", style_table_cell),
            Paragraph("Card Clima completa: cambio modalità (Spento/Caldo/Freddo), regolazione temperatura e selezione Preset (<i>Manuale</i>, <i>Crono</i>, <i>Eco</i>, <i>Boost</i>).", style_table_cell)
        ],
        [
            Paragraph("<code>sensor.termostato_&lt;piano&gt;_ore_oggi</code>", style_table_cell),
            Paragraph("Sensore Ore", style_table_cell),
            Paragraph("Ore di accensione della caldaia oggi. Cliccandoci si apre il grafico storico nativo di Home Assistant.", style_table_cell)
        ],
        [
            Paragraph("<code>sensor.termostato_&lt;piano&gt;_ore_7gg</code>", style_table_cell),
            Paragraph("Sensore Ore", style_table_cell),
            Paragraph("Ore complessive di accensione negli ultimi 7 giorni.", style_table_cell)
        ],
        [
            Paragraph("<code>sensor.termostato_&lt;piano&gt;_temperatura</code>", style_table_cell),
            Paragraph("Sensore °C", style_table_cell),
            Paragraph("Temperatura ambiente in tempo reale con cronologia storica.", style_table_cell)
        ],
        [
            Paragraph("<code>sensor.termostato_&lt;piano&gt;_umidita</code>", style_table_cell),
            Paragraph("Sensore %", style_table_cell),
            Paragraph("Umidità dell'aria rilevata nella stanza.", style_table_cell)
        ],
        [
            Paragraph("<code>sensor.termostato_&lt;piano&gt;_pressione</code>", style_table_cell),
            Paragraph("Sensore hPa", style_table_cell),
            Paragraph("Pressione barometrica rilevata dal sensore BME280.", style_table_cell)
        ],
        [
            Paragraph("<code>sensor.termostato_&lt;piano&gt;_rugiada</code>", style_table_cell),
            Paragraph("Sensore °C", style_table_cell),
            Paragraph("Punto di rugiada per prevenire umidità e muffe.", style_table_cell)
        ],
        [
            Paragraph("<code>switch.termostato_&lt;piano&gt;_crono</code>", style_table_cell),
            Paragraph("Interruttore", style_table_cell),
            Paragraph("Attiva o disattiva la programmazione settimanale oraria.", style_table_cell)
        ],
        [
            Paragraph("<code>switch.termostato_&lt;piano&gt;_antifrost</code>", style_table_cell),
            Paragraph("Interruttore", style_table_cell),
            Paragraph("Abilita/disabilita la sicurezza antigelo a 5.0 °C.", style_table_cell)
        ],
        [
            Paragraph("<code>switch.termostato_&lt;piano&gt;_openwin</code>", style_table_cell),
            Paragraph("Interruttore", style_table_cell),
            Paragraph("Abilita/disabilita il rilevamento automatico finestra aperta.", style_table_cell)
        ],
        [
            Paragraph("<code>switch.termostato_&lt;piano&gt;_buzzer</code>", style_table_cell),
            Paragraph("Interruttore", style_table_cell),
            Paragraph("Silenzia o riattiva il bip touch (es. silenziamento notturno).", style_table_cell)
        ],
        [
            Paragraph("<code>sensor.termostato_&lt;piano&gt;_wifi_signal</code>", style_table_cell),
            Paragraph("Sensore %", style_table_cell),
            Paragraph("Potenza percentuale (0&ndash;100%) del segnale Wi-Fi ricevuto.", style_table_cell)
        ],
        [
            Paragraph("<code>sensor.termostato_&lt;piano&gt;_wifi_rssi</code>", style_table_cell),
            Paragraph("Sensore dBm", style_table_cell),
            Paragraph("Livello di intensità segnale radio (RSSI) in dBm.", style_table_cell)
        ]
    ]
    t_ha = Table(ha_entities, colWidths=[4.8 * cm, 2.2 * cm, A4[0] - 3.0 * cm - 7.0 * cm])
    t_ha.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), colors.HexColor("#0f172a")),
        ('GRID', (0,0), (-1,-1), 0.5, colors.HexColor("#cbd5e1")),
        ('ROWBACKGROUNDS', (0,1), (-1,-1), [colors.white, colors.HexColor("#f8fafc")]),
        ('VALIGN', (0,0), (-1,-1), 'TOP'),
        ('TOPPADDING', (0,0), (-1,-1), 4),
        ('BOTTOMPADDING', (0,0), (-1,-1), 4),
    ]))
    story.append(t_ha)
    story.append(Spacer(1, 14))

    # --- SEZIONE 6: PROCEDURA CAMBIO WI-FI ---
    story.append(Paragraph("6. Procedura per il Cambio della Rete Wi-Fi (Nuovo Router o Password)", style_h1))
    story.append(Paragraph(
        "Se sostituisci il modem/router di casa, cambi gestore telefonico o modifichi la password del Wi-Fi, "
        "il termostato mette a disposizione <b>due metodi semplicissimi</b> per aggiornare la connessione senza perdere alcuna impostazione:",
        style_body
    ))

    # Metodo A
    story.append(Paragraph("<b>Metodo A &mdash; Dal Display Touch (Consigliato se il nuovo Wi-Fi è già attivo):</b>", style_body_bold))
    wifi_change_steps_a = [
        "<b>1. Apri il Menu:</b> tocca il pulsante <b>[MENU]</b> in alto sul display touch del termostato per passare alla pagina impostazioni.",
        "<b>2. Attiva il Punto di Accesso:</b> tocca il pulsante rosso <b>[RESET CONFIG]</b>. Il termostato <i>non cancellerà</i> le tue impostazioni del cronotermostato, ma riattiverà immediatamente la rete Wi-Fi temporanea <b>Termostato_AP</b> (noterai la scritta <code>AP:192.168.4.1</code> in basso sullo schermo).",
        "<b>3. Connetti lo Smartphone:</b> prendi il cellulare, apri l'elenco delle reti Wi-Fi e collegati a <b>Termostato_AP</b>.",
        "<b>4. Inserisci il nuovo Wi-Fi:</b> si aprirà la schermata di configurazione (o apri il browser su <code>192.168.4.1</code>); clicca su <b>'Scansiona Reti Wi-Fi'</b>, tocca il nome del tuo nuovo Wi-Fi, scrivi la nuova password e premi <b>'Salva e Connetti al Wi-Fi'</b>.",
        "<b>5. Fatto!:</b> in 5 secondi il termostato si aggancia alla nuova rete, spegne l'AP e mostra il nuovo indirizzo IP sul display."
    ]
    for s in wifi_change_steps_a:
        story.append(Paragraph(f"&bull; {s}", style_body))
    story.append(Spacer(1, 6))

    # Metodo B
    story.append(Paragraph("<b>Metodo B &mdash; Dalla Pagina Web (Se sei ancora connesso alla vecchia rete):</b>", style_body_bold))
    wifi_change_steps_b = [
        "<b>1.</b> Apri la pagina web del termostato (digitando il suo indirizzo IP attuale su cellulare o PC).",
        "<b>2.</b> Scorri fino alla scheda <b>'Connessione Wi-Fi'</b>.",
        "<b>3.</b> Clicca su <b>'Scansiona Reti Wi-Fi'</b>, seleziona il nuovo SSID, inserisci la nuova password e premi <b>'Salva e Connetti al Wi-Fi'</b>.",
        "<b>4.</b> Il termostato si riavvierà connettendosi istantaneamente al nuovo router."
    ]
    for s in wifi_change_steps_b:
        story.append(Paragraph(f"&bull; {s}", style_body))
    story.append(Spacer(1, 6))

    wifi_note_p = Paragraph(
        "<b>Nota di Sicurezza sui Tuoi Dati:</b><br/>"
        "Durante il cambio del Wi-Fi tutti gli orari del cronotermostato, le temperature desiderate, "
        "la calibrazione del touch, il volume del buzzer e le ore di lavoro della caldaia "
        "<b>rimangono al 100% salvati e protetti</b> nella memoria flash permanente del termostato.",
        style_box_text
    )
    story.append(make_box(wifi_note_p, "#f8fafc", "#16a34a"))
    story.append(Spacer(1, 14))

    # --- SEZIONE 7: DOMANDE FREQUENTI E RISOLUZIONE PROBLEMI ---
    story.append(Paragraph("7. Domande Frequenti (FAQ) e Risoluzione Problemi", style_h1))
    
    faq_items = [
        ("Cosa succede in caso di interruzione di corrente?",
         "Niente paura: tutte le impostazioni (setpoint, modalità, orari del cronotermostato, credenziali Wi-Fi e ore caldaia) sono memorizzate nella memoria non volatile del termostato. Al ritorno dell'elettricità, il dispositivo si riconnette al Wi-Fi e riprende a funzionare esattamente come prima."),
        
        ("Posso cambiare la temperatura a mano se il cronotermostato è attivo?",
         "Sì! Se senti freddo o caldo, puoi sempre premere i tasti [+] o [-] sul display o nella app. Il termostato manterrà la tua temperatura modificata fino al successivo cambio fascia oraria programmato, quando si riallineerà all'orario impostato."),
        
        ("Cosa faccio se il termostato mostra 'Termostato_AP' e non si connette?",
         "Significa che il router Wi-Fi di casa è spento o non raggiungibile. Verifica che il modem sia acceso, oppure collegati con lo smartphone alla rete 'Termostato_AP' e ripeti la selezione della rete come descritto nella Sezione 6.")
    ]
    
    for q, a in faq_items:
        story.append(Paragraph(f"<b>D: {q}</b>", style_body_bold))
        story.append(Paragraph(f"<b>R:</b> {a}", style_body))
        story.append(Spacer(1, 4))

    # Footer note
    story.append(Spacer(1, 10))
    story.append(HRFlowable(width="100%", thickness=1, color=colors.HexColor("#cbd5e1"), spaceAfter=10))
    story.append(Paragraph(
        "<i>Manuale redatto per Utenti Domestici &bull; AZ-Touch MOD 2.8\" ESP32 &bull; Termostato Smart Multi-Piano</i>",
        ParagraphStyle('FootNote', parent=styles['Normal'], fontName='Helvetica-Oblique', fontSize=8, alignment=1, textColor=colors.HexColor("#64748b"))
    ))

    # Costruzione documento
    doc.build(story, canvasmaker=NumberedCanvas)
    print(f"PDF generato con successo: {filename}")

if __name__ == '__main__':
    out = sys.argv[1] if len(sys.argv) > 1 else "Manuale_Istruzioni_Termostato_Smart.pdf"
    build_pdf(out)

