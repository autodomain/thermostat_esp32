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
        self.setFont("Helvetica-Bold", 8)
        self.setFillColor(colors.HexColor("#0284c7"))
        
        # Header (dalla pagina 2 in poi)
        if self._pageNumber > 1:
            self.drawString(1.5 * cm, A4[1] - 1.15 * cm, "TERMOSTATO SMART ESP32 (CYD & AZ-TOUCH 2.8\")")
            self.setFont("Helvetica", 8)
            self.setFillColor(colors.HexColor("#64748b"))
            self.drawRightString(A4[0] - 1.5 * cm, A4[1] - 1.15 * cm, "Manuale d'Uso & Guida Tecnica")
            self.setStrokeColor(colors.HexColor("#0284c7"))
            self.setLineWidth(1.0)
            self.line(1.5 * cm, A4[1] - 1.30 * cm, A4[0] - 1.5 * cm, A4[1] - 1.30 * cm)
            
        # Footer
        page_text = f"Pagina {self._pageNumber} di {page_count}"
        self.setFont("Helvetica-Bold", 8)
        self.setFillColor(colors.HexColor("#0369a1"))
        self.drawString(1.5 * cm, 1.0 * cm, "Sistema Multi-Piano / Multi-Zona")
        self.setFont("Helvetica", 8)
        self.setFillColor(colors.HexColor("#64748b"))
        self.drawString(6.0 * cm, 1.0 * cm, "| Home Assistant MQTT Auto-Discovery")
        self.drawRightString(A4[0] - 1.5 * cm, 1.0 * cm, page_text)
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
        topMargin=1.7 * cm,
        bottomMargin=1.7 * cm
    )

    styles = getSampleStyleSheet()
    
    # Custom Palette
    c_primary    = colors.HexColor("#0f172a") # Slate 900
    c_secondary  = colors.HexColor("#0369a1") # Sky 700
    c_accent     = colors.HexColor("#0284c7") # Sky 600
    c_dark       = colors.HexColor("#1e293b") # Slate 800
    c_light_bg   = colors.HexColor("#f8fafc") # Slate 50
    c_card_bg    = colors.HexColor("#f1f5f9") # Slate 100
    c_border     = colors.HexColor("#cbd5e1")
    c_green      = colors.HexColor("#16a34a")
    c_red        = colors.HexColor("#dc2626")
    c_amber      = colors.HexColor("#d97706")
    c_purple     = colors.HexColor("#7c3aed")
    c_cyan       = colors.HexColor("#0891b2")
    c_teal       = colors.HexColor("#0d9488")
    
    # Typography Styles
    style_title = ParagraphStyle(
        'DocTitle',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=22,
        leading=26,
        textColor=colors.HexColor("#0369a1"),
        alignment=1, # Center
        spaceAfter=4
    )
    
    style_subtitle = ParagraphStyle(
        'DocSubTitle',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=12,
        leading=16,
        textColor=colors.HexColor("#334155"),
        alignment=1,
        spaceAfter=10
    )
    
    style_badge = ParagraphStyle(
        'Badge',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=8.5,
        leading=12,
        textColor=colors.HexColor("#0284c7"),
        alignment=1,
        spaceAfter=14
    )

    style_h1 = ParagraphStyle(
        'Heading1_Custom',
        parent=styles['Heading1'],
        fontName='Helvetica-Bold',
        fontSize=13.5,
        leading=17,
        textColor=colors.HexColor("#0f172a"),
        spaceBefore=12,
        spaceAfter=6,
        keepWithNext=True
    )
    
    style_h2 = ParagraphStyle(
        'Heading2_Custom',
        parent=styles['Heading2'],
        fontName='Helvetica-Bold',
        fontSize=11,
        leading=15,
        textColor=colors.HexColor("#0369a1"),
        spaceBefore=8,
        spaceAfter=3,
        keepWithNext=True
    )

    style_body = ParagraphStyle(
        'Body_Custom',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=8.8,
        leading=13,
        textColor=colors.HexColor("#334155"),
        spaceAfter=5
    )
    
    style_body_bold = ParagraphStyle(
        'BodyBold_Custom',
        parent=style_body,
        fontName='Helvetica-Bold',
        textColor=colors.HexColor("#0f172a")
    )

    style_box_text = ParagraphStyle(
        'BoxText',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=8.6,
        leading=13,
        textColor=colors.HexColor("#1e293b")
    )
    
    style_table_cell = ParagraphStyle(
        'TableCell',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=8.0,
        leading=11.5,
        textColor=colors.HexColor("#1e293b")
    )

    style_table_header = ParagraphStyle(
        'TableHeader',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=8.5,
        leading=12,
        textColor=colors.white
    )

    story = []

    def make_box(content_para, bg_color="#f8fafc", border_color="#0284c7"):
        t = Table([[content_para]], colWidths=[A4[0] - 3.0 * cm])
        t.setStyle(TableStyle([
            ('BACKGROUND', (0,0), (-1,-1), colors.HexColor(bg_color)),
            ('BOX', (0,0), (-1,-1), 1.2, colors.HexColor(border_color)),
            ('TOPPADDING', (0,0), (-1,-1), 6),
            ('BOTTOMPADDING', (0,0), (-1,-1), 6),
            ('LEFTPADDING', (0,0), (-1,-1), 9),
            ('RIGHTPADDING', (0,0), (-1,-1), 9),
        ]))
        return t

    def make_spia_badge(code, bg_hex, fg_hex="#ffffff"):
        return f'<font color="{bg_hex}">■</font> <b><font color="{bg_hex}">[{code}]</font></b>'

    # =========================================================================
    # COPERTINA & PRESENTAZIONE
    # =========================================================================
    story.append(Spacer(1, 4))
    story.append(Paragraph("TERMOSTATO SMART TOUCH ESP32 (CYD &amp; AZ-TOUCH)", style_title))
    story.append(Paragraph("Manuale d'Uso, Guida alle Funzioni Avanzate &amp; Integrazione Domotica", style_subtitle))
    story.append(Paragraph("<b>Hardware Supportato:</b> CYD (ESP32-2432S028R) &amp; AZ-Touch MOD 2.8\" | Sensore BME280 | Home Assistant MQTT", style_badge))
    story.append(HRFlowable(width="100%", thickness=1.5, color=colors.HexColor("#0284c7"), spaceAfter=10))

    intro_p = Paragraph(
        "<b>Gentile Utente,</b> benvenuto nella guida completa del tuo nuovo <b>Termostato Smart ESP32</b>.<br/>"
        "Questo dispositivo combina la precisione del sensore termo-igrometrico/barometrico <b>BME280</b> "
        "con la comodità di un touchscreen LCD a colori da 2.8\" e una gestione domotica avanzata tramite <b>Home Assistant</b> e <b>Web Server integrato</b>.<br/>"
        "Il firmware include nativamente <b>8 nuove funzioni intelligenti</b> di risparmio energetico, salvaguardia dell'impianto idraulico e comfort, "
        "immediatamente visibili sul display grazie alla nuova <b>dashboard a 8 spie di stato a colori</b>.",
        style_box_text
    )
    story.append(make_box(intro_p, "#f8fafc", "#0284c7"))
    story.append(Spacer(1, 8))

    # =========================================================================
    # SEZIONE 1: HARDWARE & CONNESSIONI
    # =========================================================================
    story.append(Paragraph("1. Panoramica Hardware, Connettori &amp; Collegamento Sensore", style_h1))
    story.append(Paragraph(
        "Il termostato supporta nativamente due piattaforme hardware. Prima dell'accensione verificare i collegamenti:",
        style_body
    ))

    hw_data = [
        [Paragraph("Caratteristica", style_table_header), Paragraph("CYD (ESP32-2432S028R 'Yellow Display')", style_table_header), Paragraph("AZ-Touch MOD 2.8\"", style_table_header)],
        [
            Paragraph("<b>Alimentazione</b>", style_table_cell),
            Paragraph("Porta <b>USB-C</b> o <b>Micro-USB</b> (5V 1A standard)", style_table_cell),
            Paragraph("Morsettiera a vite 5V / 12-24V DC o USB", style_table_cell)
        ],
        [
            Paragraph("<b>Connettore Sensore BME280 (I2C)</b>", style_table_cell),
            Paragraph("<b>Connettore CN1 / P3 (JST 4-pin 1.25mm):</b><br/>"
                      "&bull; Pin 1: <b>GND</b> (Nero)<br/>"
                      "&bull; Pin 2: <b>3.3V</b> (Rosso)<br/>"
                      "&bull; Pin 3: <b>SDA &rarr; GPIO 22</b> (Verde/Giallo)<br/>"
                      "&bull; Pin 4: <b>SCL &rarr; GPIO 27</b> (Bianco/Blu)", style_table_cell),
            Paragraph("<b>Morsetti I2C a vite onboard:</b><br/>"
                      "&bull; <b>SDA &rarr; GPIO 17</b><br/>"
                      "&bull; <b>SCL &rarr; GPIO 16</b><br/>"
                      "&bull; Alimentazione 3.3V e GND dai pin dedicati", style_table_cell)
        ],
        [
            Paragraph("<b>Uscita Caldaia (Relè)</b>", style_table_cell),
            Paragraph("Modulo relè esterno optoisolato collegabile su <b>GPIO 22/27</b> o relè wireless via MQTT / Home Assistant", style_table_cell),
            Paragraph("Relè a contatto pulito a bordo scheda su <b>GPIO 26</b> (max 250V 3A)", style_table_cell)
        ],
        [
            Paragraph("<b>Segnalazione Acustica &amp; Ottica</b>", style_table_cell),
            Paragraph("Speaker/DAC su <b>GPIO 26</b> + LED RGB onboard (Rosso=Caldo, Blu=Freddo)", style_table_cell),
            Paragraph("Piezo buzzer integrato su <b>GPIO 21</b>", style_table_cell)
        ]
    ]
    t_hw = Table(hw_data, colWidths=[3.6 * cm, 7.5 * cm, A4[0] - 3.0 * cm - 11.1 * cm])
    t_hw.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), colors.HexColor("#0f172a")),
        ('GRID', (0,0), (-1,-1), 0.5, colors.HexColor("#cbd5e1")),
        ('ROWBACKGROUNDS', (0,1), (-1,-1), [colors.white, colors.HexColor("#f8fafc")]),
        ('VALIGN', (0,0), (-1,-1), 'TOP'),
        ('TOPPADDING', (0,0), (-1,-1), 4),
        ('BOTTOMPADDING', (0,0), (-1,-1), 4),
    ]))
    story.append(t_hw)
    story.append(Spacer(1, 8))

    # =========================================================================
    # SEZIONE 2: PRIMA ACCENSIONE E WIFI
    # =========================================================================
    story.append(Paragraph("2. Prima Accensione &amp; Connessione Wi-Fi Facile", style_h1))
    story.append(Paragraph(
        "Al primo avvio, o in assenza di rete conosciuta, il dispositivo crea una rete Wi-Fi di configurazione guidata:",
        style_body
    ))
    
    wifi_steps = [
        "<b>1. Connessione dallo Smartphone/PC:</b> apri le impostazioni Wi-Fi e collegati alla rete aperta <b>Termostato_AP</b>.",
        "<b>2. Portale Automatico:</b> si aprirà automaticamente il portale di benvenuto. Se non si apre, naviga su <b>http://192.168.4.1</b>.",
        "<b>3. Scansione Reti:</b> premi <b>'Scansiona Reti Wi-Fi'</b>, tocca il nome del tuo Wi-Fi, digita la password e premi <b>'Salva e Connetti'</b>.",
        "<b>4. Pronto all'Uso:</b> il termostato si collega in pochi secondi, mostra l'indirizzo IP locale (es. <code>192.168.1.150</code>) e sincronizza data e ora esatte via internet (NTP)."
    ]
    for step in wifi_steps:
        story.append(Paragraph(f"&bull; {step}", style_body))
    story.append(Spacer(1, 6))

    # =========================================================================
    # SEZIONE 3: DASHBOARD SPIE A COLORI SUL DISPLAY TFT
    # =========================================================================
    story.append(PageBreak())
    story.append(Paragraph("3. Nuova Dashboard Display LCD: Le 8 Spie di Stato a Colori", style_h1))
    story.append(Paragraph(
        "Nella parte superiore dello schermo LCD (subito sotto la data/ora e sopra la linea orizzontale, da <code>X=5</code> a <code>X=202</code>), "
        "è collocata la nuova <b>Dashboard delle 8 Spie di Stato</b>. Ciascuna spia misura 23x13 pixel con angoli arrotondati e adotta un'icona testuale "
        "e una combinazione cromatica precisa che riflette in tempo reale l'attivazione o l'allarme di ogni funzione:",
        style_body
    ))

    spie_data = [
        [
            Paragraph("Spia", style_table_header),
            Paragraph("Funzione &amp; Descrizione", style_table_header),
            Paragraph("Stato Spento / Inattivo", style_table_header),
            Paragraph("Stato Armato / Attivo", style_table_header),
            Paragraph("Stato Allarme / In Corso", style_table_header)
        ],
        [
            Paragraph("<b><font color='#16a34a'>[CRN]</font></b>", style_table_cell),
            Paragraph("<b>Cronotermostato Settimanale</b><br/>Programmazione oraria a fasce", style_table_cell),
            Paragraph("<font color='#475569'>Navy Scuro</font><br/>Modo Manuale", style_table_cell),
            Paragraph("<font color='#16a34a'><b>Verde Brillante</b></font><br/>Orari attivi", style_table_cell),
            Paragraph("—", style_table_cell)
        ],
        [
            Paragraph("<b><font color='#0284c7'>[GEL]</font></b>", style_table_cell),
            Paragraph("<b>Protezione Antigelo</b><br/>Sicurezza antigelo condutture", style_table_cell),
            Paragraph("<font color='#475569'>Navy Scuro</font><br/>Disabilitata", style_table_cell),
            Paragraph("<font color='#0284c7'><b>Azzurro Cielo</b></font><br/>Vigile (&le; 5.0 °C)", style_table_cell),
            Paragraph("<font color='#dc2626'><b>Rosso Allarme</b></font><br/>Caldaia forzata ON", style_table_cell)
        ],
        [
            Paragraph("<b><font color='#10b981'>[WIN]</font></b>", style_table_cell),
            Paragraph("<b>Rilevamento Finestra Aperta</b><br/>Salto termico locale o sensore MQTT", style_table_cell),
            Paragraph("<font color='#475569'>Navy Scuro</font><br/>Disabilitato", style_table_cell),
            Paragraph("<font color='#10b981'><b>Verde Acqua</b></font><br/>Monitoraggio ON", style_table_cell),
            Paragraph("<font color='#d97706'><b>Ambra/Arancio</b></font><br/>Finestra aperta (Pausa)", style_table_cell)
        ],
        [
            Paragraph("<b><font color='#0891b2'>[PMP]</font></b>", style_table_cell),
            Paragraph("<b>Antiblocco Pompa / Valvole</b><br/>Ciclo anti-grippaggio settimanale", style_table_cell),
            Paragraph("<font color='#475569'>Navy Scuro</font><br/>Disabilitato", style_table_cell),
            Paragraph("<font color='#0891b2'><b>Ciano Vivo</b></font><br/>Protezione armata", style_table_cell),
            Paragraph("<font color='#ea580c'><b>Arancio Fuoco</b></font><br/>Test 60s in corso", style_table_cell)
        ],
        [
            Paragraph("<b><font color='#7c3aed'>[NIT]</font></b>", style_table_cell),
            Paragraph("<b>Modalità Notte &amp; Auto-Dimming</b><br/>Spegnimento display programmato", style_table_cell),
            Paragraph("<font color='#475569'>Navy Scuro</font><br/>Disabilitata", style_table_cell),
            Paragraph("<font color='#7c3aed'><b>Viola</b></font><br/>Orari notte armati", style_table_cell),
            Paragraph("<font color='#4338ca'><b>Blu Notte</b></font><br/>Fascia notturna (Dimmed)", style_table_cell)
        ],
        [
            Paragraph("<b><font color='#0d9488'>[IST]</font></b>", style_table_cell),
            Paragraph("<b>Isteresi Dinamica Regolabile</b><br/>Protezione cicli caldaia (0.1..2.0 °C)", style_table_cell),
            Paragraph("<font color='#475569'>Navy Scuro</font><br/>Isteresi base fissa", style_table_cell),
            Paragraph("<font color='#0d9488'><b>Teal / Petrolio</b></font><br/>Isteresi dinamica attiva", style_table_cell),
            Paragraph("—", style_table_cell)
        ],
        [
            Paragraph("<b><font color='#ca8a04'>[BUZ]</font></b>", style_table_cell),
            Paragraph("<b>Feedback Acustico Buzzer</b><br/>Suono tocco touch e allarmi", style_table_cell),
            Paragraph("<font color='#475569'>Navy Scuro</font><br/>Muto (Vol. 0)", style_table_cell),
            Paragraph("<font color='#ca8a04'><b>Giallo Oro</b></font><br/>Buzzer attivo (Vol 1..3)", style_table_cell),
            Paragraph("—", style_table_cell)
        ],
        [
            Paragraph("<b><font color='#0284c7'>[MQT]</font></b>", style_table_cell),
            Paragraph("<b>Broker Home Assistant / MQTT</b><br/>Stato connessione domotica", style_table_cell),
            Paragraph("<font color='#475569'>Navy Scuro</font><br/>Non configurato", style_table_cell),
            Paragraph("<font color='#0284c7'><b>Sky Blue HA</b></font><br/>Connesso al broker", style_table_cell),
            Paragraph("<font color='#dc2626'><b>Rosso Allarme</b></font><br/>Server impostato ma OFFLINE", style_table_cell)
        ]
    ]

    t_spie = Table(spie_data, colWidths=[1.8 * cm, 4.8 * cm, 3.2 * cm, 3.4 * cm, A4[0] - 3.0 * cm - 13.2 * cm])
    t_spie.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), colors.HexColor("#0f172a")),
        ('GRID', (0,0), (-1,-1), 0.5, colors.HexColor("#cbd5e1")),
        ('ROWBACKGROUNDS', (0,1), (-1,-1), [colors.white, colors.HexColor("#f8fafc")]),
        ('VALIGN', (0,0), (-1,-1), 'MIDDLE'),
        ('TOPPADDING', (0,0), (-1,-1), 3.5),
        ('BOTTOMPADDING', (0,0), (-1,-1), 3.5),
        ('ALIGN', (0,0), (0,-1), 'CENTER'),
    ]))
    story.append(t_spie)
    story.append(Spacer(1, 8))

    tech_spie_p = Paragraph(
        "<b>Tecnologia Anti-Flicker &amp; Temperatura Esterna a Schermo:</b><br/>"
        "&bull; <b>Spie Intelligenti:</b> il display TFT aggiorna solo ed esclusivamente il badge il cui stato &egrave; variato "
        "(zero sfarfallio e zero impatto sulle prestazioni grafiche). Ogni modifica applicata da Web UI o da Home Assistant "
        "viene riflessa sul display in meno di 50 millisecondi.<br/>"
        "&bull; <b>Temperatura Esterna sul Display (<code>T --.- &deg;C</code>):</b> a destra della data &egrave; collocato il segnaposto "
        "della temperatura esterna, illuminato in ciano brillante. Pu&ograve; essere impostato liberamente da Home Assistant tramite topic MQTT "
        "(es. da stazione meteo, OpenWeatherMap o sensore wireless esterno). In assenza di dati visualizza il segnaposto <code>T --.- &deg;C</code>.",
        style_box_text
    )
    story.append(make_box(tech_spie_p, "#f1f5f9", "#0284c7"))
    story.append(Spacer(1, 8))

    # =========================================================================
    # SEZIONE 4: INTERFACCIA WEB CON CARD RICHIUDIBILI
    # =========================================================================
    story.append(Paragraph("4. Interfaccia Web Moderna con Card Richiudibili (Accordion)", style_h1))
    story.append(Paragraph(
        "Accedendo all'indirizzo IP del termostato da browser (es. <code>http://192.168.1.150</code>), l'interfaccia si presenta "
        "con una struttura ultra-ordinata: la card principale dello <b>Stato Termostato</b> rimane sempre aperta e visibile in alto, "
        "mentre tutte le 7 sezioni di configurazione successive sono organizzate in <b>Card Richiudibili</b>:",
        style_body
    ))

    cards_desc = [
        "<b>Chevrons Geometrici Animati:</b> un indicatore a freccia triangolare ruota dolcemente di 90° all'apertura e chiusura di ogni sezione.",
        "<b>Memoria dello Stato (localStorage):</b> il browser ricorda quali schede hai lasciato aperte o chiuse tra una visita e l'altra.",
        "<b>Card 2 &mdash; Cronotermostato Settimanale Avanzato:</b><br/>"
        "&bull; <b>Pulsante 'Salva e Attiva Crono' (Verde):</b> salva gli orari e attiva immediatamente la programmazione (spia <code>CRN</code> verde).<br/>"
        "&bull; <b>Pulsante 'Salva (Solo Manuale)' (Grigio/Azzurro):</b> salva le modifiche agli orari ma lascia il termostato in modalità manuale libera.<br/>"
        "&bull; <b>Pulsante 'Ferma Crono' (Ambra):</b> disattiva istantaneamente il cronotermostato tornando in modalità manuale con setpoint libero.",
        "<b>Card 3 &mdash; Meteo Barometrico &amp; Comfort:</b> punto di rugiada, indice Humidex di benessere termico e pressione atmosferica a 3 ore.",
        "<b>Card 4 &mdash; Funzioni Avanzate &amp; Sicurezza:</b> cursori e toggle per Isteresi (0.1..2.0 °C), Protezione Pompa, Modalità Notte, Blocco Pulizia, Sensore Finestra Esterno e <b>Temperatura Esterna MQTT</b> (per visualizzare <code>T 18.5 &deg;C</code> sul display).",
        "<b>Card 5 &mdash; Consumi &amp; Storico Caldaia:</b> ore bruciate oggi e cronologia dettagliata giorno per giorno degli ultimi 7 giorni.",
        "<b>Card 6 &mdash; Connessione Wi-Fi:</b> scansione reti in tempo reale con intensità RSSI e connessione a nuovo router senza perdere i dati.",
        "<b>Card 7 &mdash; Configurazione MQTT &amp; Multi-Piano:</b> selezione intuitiva a pillole di Casa, Piano e Stanza con topic auto-generati."
    ]
    for cd in cards_desc:
        story.append(Paragraph(f"&bull; {cd}", style_body))
    story.append(Spacer(1, 8))

    # =========================================================================
    # SEZIONE 5: DETTAGLIO FUNZIONI INTELLIGENTI
    # =========================================================================
    story.append(PageBreak())
    story.append(Paragraph("5. Guida Approfondita alle 8 Funzioni Intelligenti", style_h1))

    # 5.1 Isteresi Dinamica
    story.append(Paragraph("5.1 Isteresi Termica Regolabile (0.1 °C &ndash; 2.0 °C)", style_h2))
    story.append(Paragraph(
        "Negli impianti a radiatori tradizionali, un'isteresi troppo stretta (es. 0.1 °C) rischia di accendere e spegnere la caldaia ogni pochi minuti, "
        "usurando la scheda elettronica e bruciando gas a vuoto. Con il cursore dell'<b>Isteresi Dinamica</b> puoi scegliere il valore ideale "
        "(consigliato: <b>0.2 °C per termosifoni</b>, <b>0.4 °C per pavimenti radianti</b> o <b>pompe di calore</b>). "
        "Inoltre, un comodo interruttore permette di disattivarla tornando alla commutazione standard.",
        style_body
    ))
    story.append(Spacer(1, 4))

    # 5.2 Antiblocco Pompa
    story.append(Paragraph("5.2 Protezione Antiblocco Pompa di Circolazione &amp; Valvole (Anti-Seize)", style_h2))
    story.append(Paragraph(
        "Durante la primavera e l'estate i riscaldamenti rimangono spenti per mesi: il calcare e i depositi nei tubi causano frequentemente "
        "il grippaggio della pompa o delle elettrovalvole di zona alla prima riaccensione autunnale.<br/>"
        "La funzione <b>Antiblocco</b> controlla se il relè non è stato attivato negli ultimi 7 giorni; in tal caso, ogni mercoledì alle 12:00 "
        "esegue automaticamente un <b>ciclo di lavaggio e sblocco di 60 secondi</b> (spia <code>PMP</code> arancione). "
        "Dalla pagina Web e da Home Assistant è disponibile anche il pulsante per avviare il test manuale immediato.",
        style_body
    ))
    story.append(Spacer(1, 4))

    # 5.3 Modalita Notte
    story.append(Paragraph("5.3 Modalità Notte &amp; Auto-Dimming Programmato", style_h2))
    story.append(Paragraph(
        "Ideale quando il termostato è installato in camera da letto o in corridoio: puoi impostare la fascia notturna (es. dalle <b>23:00 alle 06:00</b>). "
        "Durante queste ore la retroilluminazione del display si spegne completamente (spia <code>NIT</code> blu notte). "
        "Al minimo tocco su qualsiasi punto dello schermo, il display si riaccende dolcemente per 15 secondi per consentire la lettura o la regolazione, "
        "per poi rispegnarsi automaticamente.",
        style_body
    ))
    story.append(Spacer(1, 4))

    # 5.4 Finestra Aperta Locale & Esterna
    story.append(Paragraph("5.4 Rilevamento Finestra Aperta: Sensore Termico Locale + Sensore Esterno MQTT", style_h2))
    story.append(Paragraph(
        "Il termostato offre due livelli di protezione per non disperdere calore durante il ricambio d'aria:<br/>"
        "&bull; <b>Sensore Locale (Salto Termico):</b> calcola la derivata della temperatura ogni 30s su una finestra di 5 minuti. "
        "Se rileva una caduta brusca (&ge; 1.5 °C), sospende il riscaldamento per 20 minuti (spia <code>WIN</code> ambra).<br/>"
        "&bull; <b>Sensore Esterno via MQTT:</b> se possiedi già un sensore magnetico porta/finestra (es. Zigbee2MQTT, Shelly o Sonoff), "
        "puoi inserire il suo topic (es. <code>home/sensori/finestra_soggiorno</code>). Appena apri la finestra, il termostato va in pausa istantanea "
        "e riprende non appena la finestra viene richiusa.",
        style_body
    ))
    story.append(Spacer(1, 4))

    # 5.5 Blocco Schermo Pulizia
    story.append(Paragraph("5.5 Blocco Schermo Pulizia / Child Lock (30 Secondi)", style_h2))
    story.append(Paragraph(
        "Pulire il display touch con un panno spesso causava variazioni accidentali di setpoint o modalità. "
        "Attivando la funzione <b>Pulisci Schermo</b> (da Web UI o Home Assistant), il touch viene congelato per 30 secondi con un countdown visibile "
        "sul display (<code>LCK 30</code>, <code>LCK 29</code>...), permettendo di passare il panno in totale tranquillità.",
        style_body
    ))
    story.append(Spacer(1, 4))

    # 5.6 Protezione Antigelo Personalizzabile
    story.append(Paragraph("5.6 Soglia di Sicurezza Antigelo Regolabile (3.0 °C &ndash; 10.0 °C)", style_h2))
    story.append(Paragraph(
        "Se ti allontani da casa o metti il termostato su Spento, l'algoritmo di guardia termica rimane sempre attivo. "
        "La soglia standard è di 5.0 °C, ma puoi personalizzarla (es. a 7.0 °C per seconde case in montagna) per evitare il gelo nei tubi. "
        "In allarme la caldaia parte a potenza massima e la spia <code>GEL</code> si illumina di rosso acceso.",
        style_body
    ))
    story.append(Spacer(1, 8))

    # =========================================================================
    # SEZIONE 6: INTEGRAZIONE HOME ASSISTANT COMPLETA
    # =========================================================================
    story.append(Paragraph("6. Integrazione con Home Assistant (MQTT Auto-Discovery Completo)", style_h1))
    story.append(Paragraph(
        "Grazie al protocollo MQTT Auto-Discovery, tutte le entità compaiono automaticamente in Home Assistant con nomi in italiano, "
        "icone dedicate e assegnazione immediata all'Area (Piano e Stanza) di competenza:",
        style_body
    ))

    ha_entities = [
        [Paragraph("Entità Home Assistant", style_table_header), Paragraph("Tipo", style_table_header), Paragraph("Funzione &amp; Utilizzo Pratico", style_table_header)],
        [
            Paragraph("<code>climate.termostato_&lt;stanza&gt;</code>", style_table_cell),
            Paragraph("Termostato", style_table_cell),
            Paragraph("Card Clima completa: cambio modalità (Spento/Caldo/Freddo), regolazione temperatura e selezione Preset (<i>Manuale</i>, <i>Crono</i>, <i>Eco</i>, <i>Boost</i>).", style_table_cell)
        ],
        [
            Paragraph("<code>switch.termostato_&lt;stanza&gt;_crono</code>", style_table_cell),
            Paragraph("Switch", style_table_cell),
            Paragraph("Attiva o disattiva la programmazione settimanale oraria (spia <code>CRN</code>).", style_table_cell)
        ],
        [
            Paragraph("<code>switch.termostato_&lt;stanza&gt;_antifrost</code>", style_table_cell),
            Paragraph("Switch", style_table_cell),
            Paragraph("Abilita/disabilita la sorveglianza antigelo (spia <code>GEL</code>).", style_table_cell)
        ],
        [
            Paragraph("<code>number.termostato_&lt;stanza&gt;_antifrost_th</code>", style_table_cell),
            Paragraph("Cursore", style_table_cell),
            Paragraph("Regola la temperatura minima di allarme antigelo (3.0..10.0 °C).", style_table_cell)
        ],
        [
            Paragraph("<code>switch.termostato_&lt;stanza&gt;_openwin</code>", style_table_cell),
            Paragraph("Switch", style_table_cell),
            Paragraph("Abilita il rilevamento locale finestra aperta da salto termico rapido.", style_table_cell)
        ],
        [
            Paragraph("<code>switch.termostato_&lt;stanza&gt;_ext_window</code>", style_table_cell),
            Paragraph("Switch", style_table_cell),
            Paragraph("Collega il sensore magnetico porta/finestra wireless via MQTT.", style_table_cell)
        ],
        [
            Paragraph("<code>sensor.termostato_&lt;stanza&gt;_temp_esterna</code>", style_table_cell),
            Paragraph("Sensore °C", style_table_cell),
            Paragraph("Temperatura esterna visualizzata a schermo (inviata da HA a <code>~outdoor_temp/set</code>).", style_table_cell)
        ],
        [
            Paragraph("<code>switch.termostato_&lt;stanza&gt;_hysteresis</code>", style_table_cell),
            Paragraph("Switch", style_table_cell),
            Paragraph("Abilita/disabilita l'isteresi dinamica regolabile (spia <code>IST</code>).", style_table_cell)
        ],
        [
            Paragraph("<code>number.termostato_&lt;stanza&gt;_hysteresis_val</code>", style_table_cell),
            Paragraph("Cursore", style_table_cell),
            Paragraph("Imposta l'isteresi da 0.1 a 2.0 °C (a scatti di 0.1 °C).", style_table_cell)
        ],
        [
            Paragraph("<code>switch.termostato_&lt;stanza&gt;_pump_prot</code>", style_table_cell),
            Paragraph("Switch", style_table_cell),
            Paragraph("Abilita la protezione antiblocco pompa settimanale (spia <code>PMP</code>).", style_table_cell)
        ],
        [
            Paragraph("<code>button.termostato_&lt;stanza&gt;_pump_test</code>", style_table_cell),
            Paragraph("Pulsante", style_table_cell),
            Paragraph("Avvia manualmente il test di sblocco pompa e valvole per 60 secondi.", style_table_cell)
        ],
        [
            Paragraph("<code>switch.termostato_&lt;stanza&gt;_night_mode</code>", style_table_cell),
            Paragraph("Switch", style_table_cell),
            Paragraph("Attiva l'auto-dimming e spegnimento display nelle ore notturne (spia <code>NIT</code>).", style_table_cell)
        ],
        [
            Paragraph("<code>button.termostato_&lt;stanza&gt;_clean_screen</code>", style_table_cell),
            Paragraph("Pulsante", style_table_cell),
            Paragraph("Attiva il blocco pulizia display touch per 30 secondi.", style_table_cell)
        ],
        [
            Paragraph("<code>sensor.termostato_&lt;stanza&gt;_ore_oggi</code>", style_table_cell),
            Paragraph("Sensore Ore", style_table_cell),
            Paragraph("Ore di lavoro effettive della caldaia oggi (grafico storico consumi).", style_table_cell)
        ],
        [
            Paragraph("<code>sensor.termostato_&lt;stanza&gt;_ore_7gg</code>", style_table_cell),
            Paragraph("Sensore Ore", style_table_cell),
            Paragraph("Ore complessive di accensione degli ultimi 7 giorni.", style_table_cell)
        ],
        [
            Paragraph("<code>sensor.termostato_&lt;stanza&gt;_wifi_signal</code>", style_table_cell),
            Paragraph("Sensore %", style_table_cell),
            Paragraph("Intensità percentuale segnale Wi-Fi con RSSI in dBm.", style_table_cell)
        ]
    ]
    t_ha = Table(ha_entities, colWidths=[4.9 * cm, 2.0 * cm, A4[0] - 3.0 * cm - 6.9 * cm])
    t_ha.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), colors.HexColor("#0f172a")),
        ('GRID', (0,0), (-1,-1), 0.5, colors.HexColor("#cbd5e1")),
        ('ROWBACKGROUNDS', (0,1), (-1,-1), [colors.white, colors.HexColor("#f8fafc")]),
        ('VALIGN', (0,0), (-1,-1), 'TOP'),
        ('TOPPADDING', (0,0), (-1,-1), 3.5),
        ('BOTTOMPADDING', (0,0), (-1,-1), 3.5),
    ]))
    story.append(t_ha)
    story.append(Spacer(1, 8))

    # =========================================================================
    # SEZIONE 7: RISOLUZIONE PROBLEMI & FAQ
    # =========================================================================
    story.append(PageBreak())
    story.append(Paragraph("7. Domande Frequenti (FAQ) &amp; Risoluzione Problemi", style_h1))
    
    faq_items = [
        ("Cosa fare se il display mostra 'Nessuna rete trovata' durante la scansione Wi-Fi?",
         "Nelle versioni precedenti il chip radio ESP32 rimaneva impegnato nei tentativi di connessione in background. "
         "Il firmware attuale disconnette automaticamente il canale prima della scansione. "
         "Se non compaiono reti, verifica che il router trasmetta sui <b>2.4 GHz</b> (l'ESP32 non supporta i 5 GHz) e premi nuovamente 'Scansiona'."),
        
        ("Cosa succede se salta la corrente o spengo il termostato?",
         "Tutte le impostazioni (orari crono, setpoint, credenziali Wi-Fi, stato abilitazione delle 8 funzioni e storico ore) "
         "sono salvate nella memoria Flash non-volatile permanente dell'ESP32. Al ritorno della corrente riparte tutto da solo senza alcuna perdita."),
        
        ("Come posso pulire il vetro del display senza sballare i parametri?",
         "Tocca il tasto 'Pulisci Schermo' nella pagina Web o su Home Assistant: il display mostrerà il conto alla rovescia 'LCK 30' "
         "e ignorerà ogni tocco per 30 secondi, dopodiché tornerà operativo con un bip di conferma."),

        ("Come faccio a sapere se il sensore BME280 è collegato bene?",
         "Apri la card 'Diagnostica &amp; Scanner I2C' nella pagina Web e premi 'Avvia Scansione I2C'. "
         "Il termostato analizzerà automaticamente tutte le coppie di pin GPIO (incluso il connettore CN1/P3 sui pin 22/27) "
         "e ti confermerà l'indirizzo trovato (solitamente <code>0x76</code>) e i valori istantanei di T, H e P."),

        ("Come passo da programmazione oraria a manuale?",
         "Dalla pagina Web nella Card 'Programmazione Oraria' basta cliccare su <b>'Ferma Crono'</b>. "
         "La spia <code>CRN</code> sul display si spegnerà (navy scuro) e potrai regolare liberamente la temperatura con i tasti [+] e [-].")
    ]
    
    for q, a in faq_items:
        story.append(Paragraph(f"<b>D: {q}</b>", style_body_bold))
        story.append(Paragraph(f"<b>R:</b> {a}", style_body))
        story.append(Spacer(1, 4))

    # Footer note
    story.append(Spacer(1, 10))
    story.append(HRFlowable(width="100%", thickness=1, color=colors.HexColor("#0284c7"), spaceAfter=8))
    story.append(Paragraph(
        "<i>Manuale Ufficiale di Istruzione e Manutenzione &bull; Termostato Smart ESP32 (CYD &amp; AZ-Touch) &bull; Rilascio v2.4 (2026)</i>",
        ParagraphStyle('FootNote', parent=styles['Normal'], fontName='Helvetica-Oblique', fontSize=8, alignment=1, textColor=colors.HexColor("#64748b"))
    ))

    # Costruzione documento
    doc.build(story, canvasmaker=NumberedCanvas)
    print(f"PDF generato con successo: {filename}")

if __name__ == '__main__':
    out = sys.argv[1] if len(sys.argv) > 1 else "Manuale_Istruzioni_Termostato_Smart.pdf"
    build_pdf(out)
