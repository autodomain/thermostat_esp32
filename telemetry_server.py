import os
import sys
import json
import sqlite3
import datetime
import threading
import time
import urllib.request
from http.server import ThreadingHTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse, parse_qs

DB_PATH = "/home/pi/iot_hub/telemetry/data/telemetry.db"
PORT = 8088

def init_db():
    os.makedirs(os.path.dirname(DB_PATH), exist_ok=True)
    conn = sqlite3.connect(DB_PATH, timeout=10.0)
    cur = conn.cursor()
    cur.execute('PRAGMA journal_mode=WAL;')
    cur.execute('''
    CREATE TABLE IF NOT EXISTS devices (
        mac TEXT PRIMARY KEY,
        device_type TEXT DEFAULT 'thermostat',
        model TEXT,
        version TEXT,
        last_ip TEXT,
        node_id INTEGER,
        gateway_id TEXT,
        first_seen TEXT,
        last_seen TEXT,
        last_temp REAL,
        last_target REAL,
        last_humidity REAL,
        last_heating INTEGER,
        last_rssi INTEGER,
        last_uptime INTEGER,
        last_heap INTEGER,
        extra_json TEXT,
        total_packets INTEGER DEFAULT 0
    )
    ''')
    cur.execute('''
    CREATE TABLE IF NOT EXISTS telemetry_history (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        mac TEXT,
        device_type TEXT,
        timestamp TEXT,
        temp REAL,
        target_temp REAL,
        humidity REAL,
        heating INTEGER,
        mode INTEGER,
        rssi INTEGER,
        free_heap INTEGER,
        uptime_s INTEGER,
        extra_json TEXT
    )
    ''')
    cur.execute('CREATE INDEX IF NOT EXISTS idx_hist_mac ON telemetry_history(mac, timestamp)')
    
    # Auto-migration of columns if table already exists
    cur.execute("PRAGMA table_info(devices)")
    cols = [r[1] for r in cur.fetchall()]
    if "device_type" not in cols:
        cur.execute("ALTER TABLE devices ADD COLUMN device_type TEXT DEFAULT 'thermostat'")
    if "node_id" not in cols:
        cur.execute("ALTER TABLE devices ADD COLUMN node_id INTEGER")
    if "gateway_id" not in cols:
        cur.execute("ALTER TABLE devices ADD COLUMN gateway_id TEXT")
    if "extra_json" not in cols:
        cur.execute("ALTER TABLE devices ADD COLUMN extra_json TEXT")

    cur.execute("PRAGMA table_info(telemetry_history)")
    hist_cols = [r[1] for r in cur.fetchall()]
    if "device_type" not in hist_cols:
        cur.execute("ALTER TABLE telemetry_history ADD COLUMN device_type TEXT")
    if "extra_json" not in hist_cols:
        cur.execute("ALTER TABLE telemetry_history ADD COLUMN extra_json TEXT")

    conn.commit()
    conn.close()

HTML_DASHBOARD = '''<!DOCTYPE html>
<html lang="it">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Autodomain IoT Fleet & Telemetry Center</title>
    <style>
        :root {
            --bg: #0b1120;
            --card-bg: #1e293b;
            --card-header: #172033;
            --border: #334155;
            --border-light: rgba(51, 65, 85, 0.6);
            --accent: #38bdf8;
            --accent-glow: rgba(56, 189, 248, 0.2);
            --text: #f8fafc;
            --text-muted: #94a3b8;
            --online: #22c55e;
            --offline: #64748b;
            --heating: #f97316;
            --io16-accent: #a855f7;
            --gw-accent: #3b82f6;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Oxygen, Ubuntu, Cantarell, sans-serif;
            background: var(--bg);
            color: var(--text);
            padding: 24px 32px;
            min-height: 100vh;
        }
        .header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding-bottom: 20px;
            border-bottom: 1px solid var(--border);
            margin-bottom: 24px;
        }
        .logo-title h1 {
            font-size: 1.65rem;
            font-weight: 700;
            background: linear-gradient(135deg, #38bdf8 0%, #818cf8 50%, #c084fc 100%);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }
        .logo-title p {
            font-size: 0.85rem;
            color: var(--text-muted);
            margin-top: 4px;
        }
        .header-actions {
            display: flex;
            align-items: center;
            gap: 12px;
        }
        .refresh-pill {
            background: var(--card-bg);
            border: 1px solid var(--border);
            padding: 8px 16px;
            border-radius: 20px;
            font-size: 0.85rem;
            color: var(--text-muted);
            display: flex;
            align-items: center;
            gap: 8px;
        }

        /* KPI Cards Grid */
        .kpi-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(210px, 1fr));
            gap: 16px;
            margin-bottom: 30px;
        }
        .kpi-card {
            background: var(--card-bg);
            border: 1px solid var(--border);
            border-radius: 12px;
            padding: 18px;
            position: relative;
            overflow: hidden;
            box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.2);
        }
        .kpi-card::before {
            content: "";
            position: absolute;
            top: 0; left: 0; right: 0; height: 3px;
            background: var(--accent);
        }
        .kpi-card.kpi-thermo::before { background: #38bdf8; }
        .kpi-card.kpi-io16::before { background: #a855f7; }
        .kpi-card.kpi-gw::before { background: #3b82f6; }
        .kpi-card.kpi-fleet::before { background: #22c55e; }

        .kpi-title { font-size: 0.78rem; color: var(--text-muted); text-transform: uppercase; letter-spacing: 0.05em; }
        .kpi-value { font-size: 1.85rem; font-weight: 700; margin-top: 6px; }
        .kpi-sub { font-size: 0.75rem; color: var(--text-muted); margin-top: 4px; }

        /* Panels */
        .panel {
            background: var(--card-bg);
            border: 1px solid var(--border);
            border-radius: 14px;
            margin-bottom: 28px;
            overflow: hidden;
            box-shadow: 0 4px 12px rgba(0,0,0,0.15);
        }
        .panel-header {
            background: var(--card-header);
            padding: 16px 20px;
            border-bottom: 1px solid var(--border);
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        .panel-title {
            display: flex;
            align-items: center;
            gap: 10px;
            font-size: 1.1rem;
            font-weight: 600;
        }
        .panel-badge {
            font-size: 0.75rem;
            padding: 3px 8px;
            border-radius: 10px;
            background: rgba(56, 189, 248, 0.15);
            color: #38bdf8;
            font-weight: 600;
        }
        .panel-desc {
            font-size: 0.8rem;
            color: var(--text-muted);
            font-weight: normal;
        }

        /* Tables */
        table {
            width: 100%;
            border-collapse: collapse;
            text-align: left;
            font-size: 0.88rem;
        }
        th {
            background: rgba(11, 17, 32, 0.6);
            color: var(--text-muted);
            font-weight: 600;
            font-size: 0.73rem;
            text-transform: uppercase;
            letter-spacing: 0.05em;
            padding: 13px 16px;
            border-bottom: 1px solid var(--border);
        }
        td {
            padding: 13px 16px;
            border-bottom: 1px solid var(--border-light);
            vertical-align: middle;
        }
        tr:last-child td { border-bottom: none; }
        tr:hover td { background: rgba(255, 255, 255, 0.02); }

        /* Badges */
        .badge {
            display: inline-block;
            padding: 3px 9px;
            border-radius: 12px;
            font-size: 0.72rem;
            font-weight: 600;
        }
        .badge-fw { background: rgba(56, 189, 248, 0.12); color: #38bdf8; border: 1px solid rgba(56, 189, 248, 0.3); }
        .badge-online { background: rgba(34, 197, 94, 0.15); color: #4ade80; border: 1px solid rgba(34, 197, 94, 0.3); }
        .badge-offline { background: rgba(100, 116, 139, 0.15); color: #94a3b8; border: 1px solid rgba(100, 116, 139, 0.3); }
        .badge-heat-on { background: rgba(249, 115, 22, 0.2); color: #fb923c; border: 1px solid rgba(249, 115, 22, 0.4); animation: pulse 2s infinite; }
        .badge-heat-off { background: rgba(148, 163, 184, 0.1); color: #64748b; }
        .badge-io16 { background: rgba(168, 85, 247, 0.15); color: #c084fc; border: 1px solid rgba(168, 85, 247, 0.3); }
        .badge-gw { background: rgba(59, 130, 246, 0.15); color: #60a5fa; border: 1px solid rgba(59, 130, 246, 0.3); }
        @keyframes pulse { 0% { opacity: 0.7; } 50% { opacity: 1; } 100% { opacity: 0.7; } }

        .btn-hist {
            background: rgba(56, 189, 248, 0.1);
            color: var(--accent);
            border: 1px solid rgba(56, 189, 248, 0.3);
            border-radius: 6px;
            padding: 5px 11px;
            font-size: 0.78rem;
            cursor: pointer;
            transition: all 0.2s;
        }
        .btn-hist:hover {
            background: var(--accent);
            color: #0b1120;
        }
        .btn-del {
            background: rgba(239, 68, 68, 0.1);
            color: #ef4444;
            border: 1px solid rgba(239, 68, 68, 0.3);
            border-radius: 6px;
            padding: 5px 11px;
            font-size: 0.78rem;
            cursor: pointer;
            transition: all 0.2s;
        }
        .btn-del:hover {
            background: #ef4444;
            color: #ffffff;
        }
        .actions-cell {
            display: flex;
            align-items: center;
            gap: 6px;
        }
        .empty-state {
            padding: 34px 20px;
            text-align: center;
            color: var(--text-muted);
            font-size: 0.85rem;
        }
        .bits-grid {
            display: flex;
            gap: 2px;
            font-family: monospace;
            font-size: 0.72rem;
        }
        .bit-dot {
            width: 14px;
            height: 14px;
            border-radius: 3px;
            display: inline-flex;
            align-items: center;
            justify-content: center;
            font-weight: bold;
        }
        .bit-on { background: #22c55e; color: #0b1120; }
        .bit-off { background: rgba(51, 65, 85, 0.6); color: #64748b; }

        /* Modal */
        .modal {
            display: none;
            position: fixed;
            top: 0; left: 0; width: 100%; height: 100%;
            background: rgba(0, 0, 0, 0.75);
            backdrop-filter: blur(4px);
            justify-content: center;
            align-items: center;
            z-index: 999;
        }
        .modal-content {
            background: var(--card-bg);
            border: 1px solid var(--border);
            border-radius: 12px;
            width: 90%;
            max-width: 850px;
            max-height: 85vh;
            display: flex;
            flex-direction: column;
            overflow: hidden;
        }
        .modal-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 16px 20px;
            border-bottom: 1px solid var(--border);
        }
        .modal-body {
            padding: 20px;
            overflow-y: auto;
        }
        .close-btn {
            background: none;
            border: none;
            color: var(--text-muted);
            font-size: 1.5rem;
            cursor: pointer;
        }
    </style>
</head>
<body>
    <div class="header">
        <div class="logo-title">
            <h1>Autodomain IoT Fleet & Telemetry Center</h1>
            <p>Standard Telemetria Aziendale &bull; Termostati Smart &bull; Interfacce IO16 RS485 &bull; Gateway MQTT ATmega1284P</p>
        </div>
        <div class="header-actions">
            <div class="refresh-pill">
                <span style="display:inline-block; width:8px; height:8px; border-radius:50%; background:var(--online)"></span>
                Live &bull; <span id="clock">--:--:--</span>
            </div>
        </div>
    </div>

    <!-- KPI Totali Parco -->
    <div class="kpi-grid">
        <div class="kpi-card kpi-thermo">
            <div class="kpi-title">Termostati Smart ESP32</div>
            <div class="kpi-value" id="kpi-thermo-online" style="color:var(--accent);">0 / 0</div>
            <div class="kpi-sub" id="kpi-thermo-sub">Online / Totale censiti</div>
        </div>
        <div class="kpi-card kpi-io16">
            <div class="kpi-title">Interfacce IO16 (PJON)</div>
            <div class="kpi-value" id="kpi-io16-online" style="color:var(--io16-accent);">0 / 0</div>
            <div class="kpi-sub" id="kpi-io16-sub">Nodi tastiera / relè attivi</div>
        </div>
        <div class="kpi-card kpi-gw">
            <div class="kpi-title">Gateway MQTT ATmega1284P</div>
            <div class="kpi-value" id="kpi-gw-online" style="color:var(--gw-accent);">0 / 0</div>
            <div class="kpi-sub" id="kpi-gw-sub">Gateway bus RS485 attivi</div>
        </div>
        <div class="kpi-card kpi-fleet">
            <div class="kpi-title">Totale Flotta Dispositivi</div>
            <div class="kpi-value" id="kpi-total-devices" style="color:var(--online);">0</div>
            <div class="kpi-sub">Permanenti nel database (no purge)</div>
        </div>
    </div>

    <!-- SEZIONE 1: Termostati Smart -->
    <div class="panel">
        <div class="panel-header">
            <div class="panel-title">
                <span>🌡️ Termostati Smart ESP32</span>
                <span class="panel-badge" id="badge-thermo-count">0 termostati</span>
            </div>
            <div class="panel-desc">Modelli CYD 2.8″ TFT e AZ-Touch 2.4″ MOD</div>
        </div>
        <table>
            <thead>
                <tr>
                    <th>Stato</th>
                    <th>Modello</th>
                    <th>MAC Address</th>
                    <th>IP Cliente</th>
                    <th>Versione FW</th>
                    <th>Temp / Target</th>
                    <th>Umidit&agrave;</th>
                    <th>Caldaia</th>
                    <th>WiFi RSSI</th>
                    <th>Ultimo Ping</th>
                    <th>Azioni</th>
                </tr>
            </thead>
            <tbody id="thermo-rows">
                <tr><td colspan="11" class="empty-state">Caricamento termostati...</td></tr>
            </tbody>
        </table>
    </div>

    <!-- SEZIONE 2: Interfacce IO16 -->
    <div class="panel">
        <div class="panel-header">
            <div class="panel-title">
                <span>⚡ Interfacce Tastiera &amp; Rel&egrave; IO16</span>
                <span class="panel-badge" style="background:rgba(168,85,247,0.15); color:#c084fc;" id="badge-io16-count">0 nodi</span>
            </div>
            <div class="panel-desc">Nodi periferici su Bus bilanciato RS485 PJON ThroughSerial (1..63)</div>
        </div>
        <table>
            <thead>
                <tr>
                    <th>Stato</th>
                    <th>ID Nodo</th>
                    <th>Modello / MCU</th>
                    <th>Gateway Master</th>
                    <th>Bus RS485</th>
                    <th>Ingressi (8 Tasti)</th>
                    <th>Uscite (8 LED)</th>
                    <th>Temp LM75</th>
                    <th>Backlight PWM</th>
                    <th>Ultimo Ping</th>
                    <th>Azioni</th>
                </tr>
            </thead>
            <tbody id="io16-rows">
                <tr><td colspan="11" class="empty-state">In attesa di report telemetria dalle interfacce IO16...</td></tr>
            </tbody>
        </table>
    </div>

    <!-- SEZIONE 3: Gateway MQTT ATmega1284P -->
    <div class="panel">
        <div class="panel-header">
            <div class="panel-title">
                <span>🌐 Gateway Ethernet &amp; MQTT (ATmega1284P)</span>
                <span class="panel-badge" style="background:rgba(59,130,246,0.15); color:#60a5fa;" id="badge-gw-count">0 gateway</span>
            </div>
            <div class="panel-desc">Bridge Ethernet Wiznet W5500 &bull; Home Assistant MQTT &bull; Master RS485 UART0/UART1</div>
        </div>
        <table>
            <thead>
                <tr>
                    <th>Stato</th>
                    <th>Modello Gateway</th>
                    <th>MAC / IP W5500</th>
                    <th>Versione FW</th>
                    <th>Broker MQTT</th>
                    <th>Nodi IO16 Gestiti</th>
                    <th>Bus Throughput</th>
                    <th>RAM Libera</th>
                    <th>Ultimo Ping</th>
                    <th>Azioni</th>
                </tr>
            </thead>
            <tbody id="gw-rows">
                <tr><td colspan="10" class="empty-state">In attesa di report telemetria dai Gateway ATmega1284P...</td></tr>
            </tbody>
        </table>
    </div>

    <!-- Modal Storico -->
    <div class="modal" id="hist-modal">
        <div class="modal-content">
            <div class="modal-header">
                <h3 id="modal-title">Storico Telemetria</h3>
                <button class="close-btn" onclick="closeModal()">&times;</button>
            </div>
            <div class="modal-body">
                <table>
                    <thead>
                        <tr>
                            <th>Data e Ora</th>
                            <th>Tipo Dispositivo</th>
                            <th>Temp / Metriche</th>
                            <th>Stato / Rel&egrave;</th>
                            <th>WiFi / Segnale</th>
                            <th>Free Heap / RAM</th>
                            <th>Uptime</th>
                        </tr>
                    </thead>
                    <tbody id="hist-rows"></tbody>
                </table>
            </div>
        </div>
    </div>

    <script>
        function updateClock() {
            document.getElementById('clock').innerText = new Date().toLocaleTimeString();
        }
        setInterval(updateClock, 1000);
        updateClock();

        function timeAgo(dateString) {
            if (!dateString) return "Mai";
            const date = new Date(dateString.replace(' ', 'T'));
            const now = new Date();
            const sec = Math.round((now - date) / 1000);
            if (sec < 60) return `${sec}s fa`;
            if (sec < 3600) return `${Math.floor(sec/60)}m fa`;
            if (sec < 86400) return `${Math.floor(sec/3600)}h fa`;
            return `${Math.floor(sec/86400)}g fa`;
        }

        function renderBits(mask, len) {
            let html = '<div class="bits-grid">';
            for (let i = 0; i < (len || 8); i++) {
                const bit = (mask >> i) & 1;
                html += `<span class="bit-dot ${bit ? 'bit-on' : 'bit-off'}" title="Canale ${i+1}: ${bit ? 'ON' : 'OFF'}">${i+1}</span>`;
            }
            html += '</div>';
            return html;
        }

        async function loadDevices() {
            try {
                const res = await fetch('/api/devices');
                const data = await res.json();

                const thermos = data.thermostats ? data.thermostats.devices : [];
                const io16s = data.io16 ? data.io16.devices : [];
                const gws = data.gateways ? data.gateways.devices : [];

                // KPI
                document.getElementById('kpi-thermo-online').innerText = `${data.thermostats ? data.thermostats.online : 0} / ${thermos.length}`;
                document.getElementById('kpi-io16-online').innerText = `${data.io16 ? data.io16.online : 0} / ${io16s.length}`;
                document.getElementById('kpi-gw-online').innerText = `${data.gateways ? data.gateways.online : 0} / ${gws.length}`;
                document.getElementById('kpi-total-devices').innerText = data.total || 0;

                document.getElementById('badge-thermo-count').innerText = `${thermos.length} termostati`;
                document.getElementById('badge-io16-count').innerText = `${io16s.length} nodi`;
                document.getElementById('badge-gw-count').innerText = `${gws.length} gateway`;

                // Render Termostati
                const thermoTbody = document.getElementById('thermo-rows');
                if (thermos.length === 0) {
                    thermoTbody.innerHTML = `<tr><td colspan="11" class="empty-state">Nessun termostato smart rilevato finora. I termostati inviano un report automatico ogni 15 minuti via HTTP.</td></tr>`;
                } else {
                    thermoTbody.innerHTML = thermos.map(d => {
                        const onlineBadge = d.is_online 
                            ? '<span class="badge badge-online">&bull; Online</span>' 
                            : '<span class="badge badge-offline">&bull; Offline</span>';
                        const heatBadge = d.last_heating 
                            ? '<span class="badge badge-heat-on">&bull; ACCESA</span>' 
                            : '<span class="badge badge-heat-off">SPENTA</span>';
                        const hwName = d.model === 'cyd' ? 'CYD 2.8&Prime;' : (d.model === 'az_touch' ? 'AZ-Touch 2.4&Prime;' : (d.model || 'ESP32'));
                        return `
                            <tr>
                                <td>${onlineBadge}</td>
                                <td><strong>${hwName}</strong></td>
                                <td><code>${d.mac}</code></td>
                                <td>${d.last_ip || '-'}</td>
                                <td><span class="badge badge-fw">${d.version || 'v2.5.0'}</span></td>
                                <td><strong>${d.last_temp ? d.last_temp.toFixed(1) + '°C' : '-'}</strong> <span style="color:#94a3b8">/ ${d.last_target ? d.last_target.toFixed(1) + '°C' : '-'}</span></td>
                                <td>${d.last_humidity ? d.last_humidity.toFixed(0) + '%' : '-'}</td>
                                <td>${heatBadge}</td>
                                <td>${d.last_rssi ? d.last_rssi + ' dBm' : '-'}</td>
                                <td><span title="${d.last_seen}">${timeAgo(d.last_seen)}</span></td>
                                <td>
                                    <div class="actions-cell">
                                        <button class="btn-hist" onclick="viewHistory('${d.mac}')" title="Visualizza storico telemetria">Storico</button>
                                        <button class="btn-del" onclick="deleteDevice('${d.mac}')" title="Elimina dispositivo">Elimina</button>
                                    </div>
                                </td>
                            </tr>
                        `;
                    }).join('');
                }

                // Render IO16
                const io16Tbody = document.getElementById('io16-rows');
                if (io16s.length === 0) {
                    io16Tbody.innerHTML = `<tr><td colspan="11" class="empty-state">Nessuna interfaccia IO16 censita. I dati vengono raccolti dal Gateway e inoltrati a questo server.</td></tr>`;
                } else {
                    io16Tbody.innerHTML = io16s.map(d => {
                        const onlineBadge = d.is_online 
                            ? '<span class="badge badge-online">&bull; Online</span>' 
                            : '<span class="badge badge-offline">&bull; Offline</span>';
                        const extra = d.extra || {};
                        const inputsMask = extra.inputs != null ? extra.inputs : 0;
                        const outputsMask = extra.outputs != null ? extra.outputs : 0;
                        const tempLm75 = extra.temp_lm75 != null ? extra.temp_lm75.toFixed(1) + '°C' : (d.last_temp ? d.last_temp.toFixed(1) + '°C' : '-');
                        const pwmBk = extra.backlight != null ? extra.backlight + '%' : '-';
                        const bus = extra.bus != null ? `Bus ${extra.bus}` : (d.last_ip || 'RS485');
                        const nodeId = d.node_id != null ? `#${d.node_id}` : (extra.node_id != null ? `#${extra.node_id}` : d.mac);

                        return `
                            <tr>
                                <td>${onlineBadge}</td>
                                <td><strong style="color:var(--io16-accent);">${nodeId}</strong></td>
                                <td><span class="badge badge-io16">${d.model || 'IO16 Keyboard'}</span></td>
                                <td><code>${d.gateway_id || extra.gateway_id || '-'}</code></td>
                                <td>${bus}</td>
                                <td>${renderBits(inputsMask, 8)}</td>
                                <td>${renderBits(outputsMask, 8)}</td>
                                <td><strong>${tempLm75}</strong></td>
                                <td>${pwmBk}</td>
                                <td><span title="${d.last_seen}">${timeAgo(d.last_seen)}</span></td>
                                <td>
                                    <div class="actions-cell">
                                        <button class="btn-hist" onclick="viewHistory('${d.mac}')" title="Visualizza storico telemetria">Storico</button>
                                        <button class="btn-del" onclick="deleteDevice('${d.mac}')" title="Elimina interfaccia">Elimina</button>
                                    </div>
                                </td>
                            </tr>
                        `;
                    }).join('');
                }

                // Render Gateway
                const gwTbody = document.getElementById('gw-rows');
                if (gws.length === 0) {
                    gwTbody.innerHTML = `<tr><td colspan="10" class="empty-state">Nessun Gateway Ethernet ATmega1284P censito finora. Il gateway invia lo stato del broker MQTT e della matrice nodi.</td></tr>`;
                } else {
                    gwTbody.innerHTML = gws.map(d => {
                        const onlineBadge = d.is_online 
                            ? '<span class="badge badge-online">&bull; Online</span>' 
                            : '<span class="badge badge-offline">&bull; Offline</span>';
                        const extra = d.extra || {};
                        const broker = extra.mqtt_broker || (extra.mqtt_connected ? 'Connesso' : 'Disconnesso');
                        const nodesCount = extra.active_nodes != null ? `${extra.active_nodes} nodi` : (d.total_packets ? `${d.total_packets} pkt` : '-');
                        const throughput = extra.throughput || extra.bus_load || 'Normale';
                        const freeRam = extra.free_ram ? `${extra.free_ram} B` : (d.last_heap ? `${(d.last_heap/1024).toFixed(1)} KB` : '-');

                        return `
                            <tr>
                                <td>${onlineBadge}</td>
                                <td><span class="badge badge-gw">${d.model || 'ATmega1284P Gateway'}</span></td>
                                <td><code>${d.mac}</code><br><small style="color:var(--text-muted);">${d.last_ip || '-'}</small></td>
                                <td><span class="badge badge-fw">${d.version || 'v2.1'}</span></td>
                                <td><strong style="color:${extra.mqtt_connected !== false ? '#4ade80' : '#f87171'}">${broker}</strong></td>
                                <td>${nodesCount}</td>
                                <td>${throughput}</td>
                                <td>${freeRam}</td>
                                <td><span title="${d.last_seen}">${timeAgo(d.last_seen)}</span></td>
                                <td>
                                    <div class="actions-cell">
                                        <button class="btn-hist" onclick="viewHistory('${d.mac}')" title="Visualizza storico telemetria">Storico</button>
                                        <button class="btn-del" onclick="deleteDevice('${d.mac}')" title="Elimina gateway">Elimina</button>
                                    </div>
                                </td>
                            </tr>
                        `;
                    }).join('');
                }

            } catch(e) {
                console.error("Fetch error:", e);
            }
        }

        async function deleteDevice(mac) {
            if (!confirm("Sei sicuro di voler eliminare definitivamente il dispositivo " + mac + " dal database?\\nL'operazione rimuoverà la scheda e tutto il relativo storico.")) {
                return;
            }
            try {
                const res = await fetch('/api/delete_device', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ mac: mac })
                });
                const result = await res.json();
                if (result.status === 'ok') {
                    await loadDevices();
                } else {
                    alert('Errore eliminazione: ' + (result.error || 'sconosciuto'));
                }
            } catch (e) {
                alert('Errore di comunicazione: ' + e);
            }
        }

        async function viewHistory(mac) {
            document.getElementById('modal-title').innerText = `Storico Telemetria Dispositivo ${mac}`;
            const rows = document.getElementById('hist-rows');
            rows.innerHTML = `<tr><td colspan="7" class="empty-state">Caricamento storico...</td></tr>`;
            document.getElementById('hist-modal').style.display = 'flex';

            try {
                const res = await fetch(`/api/history?mac=${encodeURIComponent(mac)}&limit=50`);
                const data = await res.json();
                if (!data || data.length === 0) {
                    rows.innerHTML = `<tr><td colspan="7" class="empty-state">Nessuna rilevazione precedente registrata per questo dispositivo.</td></tr>`;
                    return;
                }
                rows.innerHTML = data.map(h => `
                    <tr>
                        <td>${h.timestamp}</td>
                        <td><span class="badge ${h.device_type === 'io16' ? 'badge-io16' : (h.device_type === 'gateway' ? 'badge-gw' : 'badge-fw')}">${h.device_type || 'thermostat'}</span></td>
                        <td><strong>${h.temp != null ? h.temp.toFixed(1) + '°C' : '-'}</strong></td>
                        <td>${h.heating ? '<span style="color:#f97316">ATTIVO</span>' : 'OFF'}</td>
                        <td>${h.rssi ? h.rssi + ' dBm' : '-'}</td>
                        <td>${h.free_heap ? (h.free_heap/1024).toFixed(0) + ' KB' : '-'}</td>
                        <td>${h.uptime_s ? h.uptime_s + ' s' : '-'}</td>
                    </tr>
                `).join('');
            } catch(e) {
                rows.innerHTML = `<tr><td colspan="7" class="empty-state" style="color:#ef4444">Errore nel caricamento storico: ${e}</td></tr>`;
            }
        }

        function closeModal() {
            document.getElementById('hist-modal').style.display = 'none';
        }
        window.onclick = function(e) {
            if (e.target === document.getElementById('hist-modal')) closeModal();
        }

        loadDevices();
        setInterval(loadDevices, 30000);
    </script>
</body>
</html>
'''

def ingest_telemetry_dict(data, client_ip="127.0.0.1"):
    mac = data.get("mac") or data.get("id")
    if not mac:
        if "node_id" in data:
            gw = data.get("gateway_id", "gw")
            mac = f"io16_{gw}_{data['node_id']}"
        else:
            return False, "Missing mac or id field"

    now_str = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%d %H:%M:%S")
    
    # Detect device_type
    dtype = data.get("device_type")
    model = data.get("model", "")
    if not dtype:
        if "io16" in model.lower() or "node_id" in data:
            dtype = "io16"
        elif "gateway" in model.lower() or "atmega" in model.lower():
            dtype = "gateway"
        else:
            dtype = "thermostat"

    version = data.get("version", "v2.5.0")
    node_id = data.get("node_id")
    gateway_id = data.get("gateway_id")
    temp = data.get("temp")
    target_temp = data.get("target_temp")
    humidity = data.get("humidity")
    heating = 1 if data.get("heating") else 0
    mode = data.get("mode", 0)
    rssi = data.get("rssi", 0)
    uptime = data.get("uptime_s", 0)
    free_heap = data.get("free_heap") or data.get("free_ram", 0)

    # Collect all extra properties
    known_keys = {"mac", "id", "device_type", "model", "version", "temp", "target_temp", 
                  "humidity", "heating", "mode", "rssi", "uptime_s", "free_heap", "free_ram", 
                  "node_id", "gateway_id"}
    extra_dict = {k: v for k, v in data.items() if k not in known_keys}
    if node_id is not None:
        extra_dict["node_id"] = node_id
    if gateway_id:
        extra_dict["gateway_id"] = gateway_id
    extra_json = json.dumps(extra_dict) if extra_dict else None

    conn = sqlite3.connect(DB_PATH, timeout=10.0)
    cur = conn.cursor()

    # Update or insert device
    cur.execute('''
    INSERT INTO devices (
        mac, device_type, model, version, last_ip, node_id, gateway_id,
        first_seen, last_seen, last_temp, last_target, last_humidity,
        last_heating, last_rssi, last_uptime, last_heap, extra_json, total_packets
    )
    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, 1)
    ON CONFLICT(mac) DO UPDATE SET
        device_type = excluded.device_type,
        model = excluded.model,
        version = excluded.version,
        last_ip = excluded.last_ip,
        node_id = excluded.node_id,
        gateway_id = excluded.gateway_id,
        last_seen = excluded.last_seen,
        last_temp = excluded.last_temp,
        last_target = excluded.last_target,
        last_humidity = excluded.last_humidity,
        last_heating = excluded.last_heating,
        last_rssi = excluded.last_rssi,
        last_uptime = excluded.last_uptime,
        last_heap = excluded.last_heap,
        extra_json = excluded.extra_json,
        total_packets = devices.total_packets + 1
    ''', (mac, dtype, model, version, client_ip, node_id, gateway_id,
          now_str, now_str, temp, target_temp, humidity, heating, rssi, uptime, free_heap, extra_json))

    # Insert history record
    cur.execute('''
    INSERT INTO telemetry_history (
        mac, device_type, timestamp, temp, target_temp, humidity,
        heating, mode, rssi, free_heap, uptime_s, extra_json
    )
    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    ''', (mac, dtype, now_str, temp, target_temp, humidity, heating, mode, rssi, free_heap, uptime, extra_json))

    # Se il payload include nodi IO16 aggregati dal Gateway, li registra o aggiorna in batch
    nodes_list = data.get("nodes")
    if isinstance(nodes_list, list):
        gw_mac = mac
        for n in nodes_list:
            if not isinstance(n, dict):
                continue
            n_id = n.get("node_id") if n.get("node_id") is not None else n.get("id")
            if n_id is None:
                continue
            node_mac = f"io16_{gw_mac}_{n_id}"
            node_model = n.get("model", "IO16 Keyboard")
            node_ver = n.get("version", version)
            node_temp = n.get("temp_lm75") if n.get("temp_lm75") is not None else n.get("temp")
            node_bus = n.get("bus", 0)
            node_inputs = n.get("inputs", 0)
            node_outputs = n.get("outputs", 0)
            node_bk = n.get("backlight", 80)

            node_is_online = bool(n.get("is_online"))
            node_extra = {
                "node_id": n_id,
                "gateway_id": gw_mac,
                "bus": node_bus,
                "inputs": node_inputs,
                "outputs": node_outputs,
                "temp_lm75": node_temp,
                "backlight": node_bk,
                "is_online": node_is_online
            }
            for k, v in n.items():
                if k not in node_extra and k not in ("node_id", "id", "model", "version"):
                    node_extra[k] = v
            cur.execute('''
            INSERT INTO devices (
                mac, device_type, model, version, last_ip, node_id, gateway_id,
                first_seen, last_seen, last_temp, last_target, last_humidity,
                last_heating, last_rssi, last_uptime, last_heap, extra_json, total_packets
            )
            VALUES (?, 'io16', ?, ?, ?, ?, ?, ?, ?, ?, NULL, NULL, 0, 0, ?, 0, ?, 1)
            ON CONFLICT(mac) DO UPDATE SET
                device_type = 'io16',
                model = excluded.model,
                version = excluded.version,
                last_ip = excluded.last_ip,
                node_id = excluded.node_id,
                gateway_id = excluded.gateway_id,
                last_seen = CASE WHEN ? = 1 THEN excluded.last_seen ELSE devices.last_seen END,
                last_temp = excluded.last_temp,
                extra_json = excluded.extra_json,
                total_packets = devices.total_packets + 1
            ''', (node_mac, node_model, node_ver, client_ip, n_id, gw_mac,
                  now_str, now_str if node_is_online else "2000-01-01 00:00:00", node_temp, n.get("uptime_s", 0), json.dumps(node_extra),
                  1 if node_is_online else 0))

    conn.commit()
    conn.close()
    return True, dtype

class TelemetryHandler(BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"
    timeout = 5

    def _send_json(self, data, code=200):
        body = json.dumps(data).encode('utf-8')
        self.send_response(code)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, DELETE, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.send_header('Connection', 'close')
        self.send_header('Content-Length', str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_OPTIONS(self):
        self.send_response(204)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, DELETE, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.send_header('Connection', 'close')
        self.end_headers()

    def do_GET(self):
        parsed = urlparse(self.path)
        path = parsed.path
        
        if path == "/" or path == "/dashboard":
            body = HTML_DASHBOARD.encode('utf-8')
            self.send_response(200)
            self.send_header('Content-Type', 'text/html; charset=utf-8')
            self.send_header('Connection', 'close')
            self.send_header('Content-Length', str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/api/devices":
            conn = sqlite3.connect(DB_PATH, timeout=10.0)
            conn.row_factory = sqlite3.Row
            cur = conn.cursor()
            cur.execute("SELECT * FROM devices ORDER BY last_seen DESC")
            raw_rows = [dict(r) for r in cur.fetchall()]
            conn.close()

            now = datetime.datetime.now(datetime.timezone.utc)
            thermostats = []
            io16_list = []
            gateways = []

            for r in raw_rows:
                is_online = False
                if r.get("last_seen"):
                    try:
                        ls = datetime.datetime.strptime(r["last_seen"], "%Y-%m-%d %H:%M:%S").replace(tzinfo=datetime.timezone.utc)
                        if (now - ls).total_seconds() < 1800: # 30 min
                            is_online = True
                    except Exception:
                        pass
                r["is_online"] = is_online
                
                # Parse extra_json if present
                extra = {}
                if r.get("extra_json"):
                    try:
                        extra = json.loads(r["extra_json"])
                    except Exception:
                        pass
                r["extra"] = extra

                # Categorizzazione
                dtype = (r.get("device_type") or "").lower()
                model = (r.get("model") or "").lower()

                if dtype == "io16" or "io16" in model or r.get("node_id") is not None:
                    is_online = bool(extra.get("is_online", False)) and is_online
                    r["is_online"] = is_online
                    io16_list.append(r)
                elif dtype in ("gateway", "gateway_atmega128") or "gateway" in model or "atmega" in model:
                    gateways.append(r)
                else:
                    thermostats.append(r)

            th_online = sum(1 for d in thermostats if d["is_online"])
            io_online = sum(1 for d in io16_list if d["is_online"])
            gw_online = sum(1 for d in gateways if d["is_online"])
            th_heating = sum(1 for d in thermostats if d.get("last_heating"))

            self._send_json({
                "total": len(raw_rows),
                "online": th_online + io_online + gw_online,
                "thermostats": {
                    "total": len(thermostats),
                    "online": th_online,
                    "heating": th_heating,
                    "devices": thermostats
                },
                "io16": {
                    "total": len(io16_list),
                    "online": io_online,
                    "devices": io16_list
                },
                "gateways": {
                    "total": len(gateways),
                    "online": gw_online,
                    "devices": gateways
                },
                # Backward-compatibility
                "heating_count": th_heating,
                "devices": raw_rows
            })
            return

        if path == "/api/history":
            qs = parse_qs(parsed.query)
            mac = qs.get("mac", [""])[0]
            limit = int(qs.get("limit", [50])[0])
            
            conn = sqlite3.connect(DB_PATH, timeout=10.0)
            conn.row_factory = sqlite3.Row
            cur = conn.cursor()
            cur.execute("SELECT * FROM telemetry_history WHERE mac = ? ORDER BY id DESC LIMIT ?", (mac, limit))
            rows = [dict(r) for r in cur.fetchall()]
            conn.close()
            self._send_json(rows)
            return

        self._send_json({"error": "Not Found"}, 404)

    def do_DELETE(self):
        parsed = urlparse(self.path)
        if parsed.path in ("/api/device", "/api/devices", "/api/delete_device"):
            qs = parse_qs(parsed.query)
            mac = qs.get("mac", [""])[0]
            if not mac:
                try:
                    content_length = int(self.headers.get('Content-Length', 0))
                    if content_length > 0:
                        post_data = self.rfile.read(content_length)
                        data = json.loads(post_data.decode('utf-8'))
                        mac = data.get("mac")
                except Exception:
                    pass

            if not mac:
                self._send_json({"error": "Missing mac parameter"}, 400)
                return

            conn = sqlite3.connect(DB_PATH, timeout=10.0)
            cur = conn.cursor()
            cur.execute("DELETE FROM devices WHERE mac = ?", (mac,))
            cur.execute("DELETE FROM telemetry_history WHERE mac = ?", (mac,))
            conn.commit()
            conn.close()
            print(f"[Telemetry] Dispositivo {mac} eliminato manualmente")
            self._send_json({"status": "ok", "deleted": mac})
            return

        self._send_json({"error": "Not Found"}, 404)

    def do_POST(self):
        parsed = urlparse(self.path)
        
        # Endpoint cancellazione singola
        if parsed.path in ("/api/delete_device", "/api/delete"):
            try:
                content_length = int(self.headers.get('Content-Length', 0))
                post_data = self.rfile.read(content_length)
                data = json.loads(post_data.decode('utf-8'))
                mac = data.get("mac")
            except Exception:
                qs = parse_qs(parsed.query)
                mac = qs.get("mac", [""])[0]

            if not mac:
                self._send_json({"error": "Missing mac field"}, 400)
                return

            conn = sqlite3.connect(DB_PATH, timeout=10.0)
            cur = conn.cursor()
            cur.execute("DELETE FROM devices WHERE mac = ?", (mac,))
            cur.execute("DELETE FROM telemetry_history WHERE mac = ?", (mac,))
            conn.commit()
            conn.close()
            print(f"[Telemetry] Dispositivo {mac} rimosso dal database")
            self._send_json({"status": "ok", "deleted": mac})
            return

        if parsed.path != "/api/telemetry":
            self._send_json({"error": "Endpoint Not Found"}, 404)
            return

        try:
            content_length = int(self.headers.get('Content-Length', 0))
            post_data = self.rfile.read(content_length)
            data = json.loads(post_data.decode('utf-8'))
        except Exception as e:
            self._send_json({"error": f"Invalid JSON payload: {e}"}, 400)
            return

        ok, res = ingest_telemetry_dict(data, self.client_address[0])
        if not ok:
            self._send_json({"error": res}, 400)
            return

        now_str = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%d %H:%M:%S")
        self._send_json({"status": "ok", "device_type": res, "received_at": now_str})

def poll_gateway_loop():
    """Thread in background che interroga periodicamente il Gateway Ethernet (192.168.1.66) e registra la telemetria."""
    print("[Telemetry] Poller Gateway Ethernet avviato (intervallo 15s)...")
    while True:
        try:
            url = "http://192.168.1.66/api/status"
            req = urllib.request.Request(url, headers={'User-Agent': 'AutodomainTelemetryHub/1.0'})
            with urllib.request.urlopen(req, timeout=4.0) as resp:
                if resp.status == 200:
                    d = json.loads(resp.read().decode('utf-8'))
                    gw_mac = "AA:ED:AB:FE:AA:F2"
                    dbg = d.get("dbg", {})
                    ram = dbg.get("ram", 4745)
                    up = dbg.get("up", 0)
                    temp = d.get("temp")
                    ptx = d.get("ptx", 0)
                    prx = d.get("prx", 0)
                    health = d.get("health", 100)
                    mqtt_conn = (d.get("mqtt") == 1)
                    
                    nds_raw = d.get("nds", [])
                    nodes_list = []
                    online_nodes_count = 0
                    for n in nds_raw:
                        if not isinstance(n, dict):
                            continue
                        nid = n.get("i")
                        if nid is None:
                            continue
                        
                        # Il flag 'o' indica se il nodo ha risposto al ping sul bus RS485 (1=online, 0=offline)
                        node_is_online = (n.get("o") == 1)
                        if node_is_online:
                            online_nodes_count += 1
                        
                        # Se il nodo è presente in memoria EEPROM del gateway ma non è MAI stato visto
                        # in questa sessione (l == 999999 o mai risposto) ed è offline (o == 0), non è collegato!
                        sec_ago = n.get("l", 999999)
                        if not node_is_online and sec_ago >= 999999:
                            continue

                        nodes_list.append({
                            "node_id": nid,
                            "model": f"IO16 Keyboard vA",
                            "version": "2.1-beta",
                            "bus": n.get("b", 1),
                            "inputs": n.get("in", 0),
                            "outputs": n.get("out", 0),
                            "temp_lm75": n.get("t"),
                            "backlight": n.get("bl", 50),
                            "quality": n.get("q", 0),
                            "is_online": node_is_online
                        })
                    
                    payload = {
                        "device_type": "gateway",
                        "mac": gw_mac,
                        "model": "ATmega1284P MQTT Gateway",
                        "version": "2.1-beta",
                        "ip": "192.168.1.66",
                        "mqtt_connected": mqtt_conn,
                        "mqtt_broker": "192.168.1.68:1883",
                        "active_nodes": online_nodes_count,
                        "throughput": f"{ptx + prx} pkt (Bus {health}%)",
                        "free_ram": ram,
                        "uptime_s": up,
                        "temp": temp,
                        "nodes": nodes_list
                    }
                    ingest_telemetry_dict(payload, client_ip="192.168.1.66")
        except Exception:
            pass
        time.sleep(15)

def main():
    init_db()

    # Avvia thread daemon per polling automatico del Gateway Ethernet
    gw_thread = threading.Thread(target=poll_gateway_loop, daemon=True)
    gw_thread.start()

    server = ThreadingHTTPServer(('0.0.0.0', PORT), TelemetryHandler)
    server.daemon_threads = True
    print(f"Telemetry server running on port {PORT} (multithreaded)...")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    server.server_close()

if __name__ == '__main__':
    main()
