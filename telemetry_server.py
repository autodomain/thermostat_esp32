import os
import sys
import json
import sqlite3
import datetime
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse, parse_qs

DB_PATH = "/home/pi/iot_hub/telemetry/data/telemetry.db"
PORT = 8088

def init_db():
    os.makedirs(os.path.dirname(DB_PATH), exist_ok=True)
    conn = sqlite3.connect(DB_PATH)
    cur = conn.cursor()
    cur.execute('''
    CREATE TABLE IF NOT EXISTS devices (
        mac TEXT PRIMARY KEY,
        model TEXT,
        version TEXT,
        last_ip TEXT,
        first_seen TEXT,
        last_seen TEXT,
        last_temp REAL,
        last_target REAL,
        last_humidity REAL,
        last_heating INTEGER,
        last_rssi INTEGER,
        last_uptime INTEGER,
        last_heap INTEGER,
        total_packets INTEGER DEFAULT 0
    )
    ''')
    cur.execute('''
    CREATE TABLE IF NOT EXISTS telemetry_history (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        mac TEXT,
        timestamp TEXT,
        temp REAL,
        target_temp REAL,
        humidity REAL,
        heating INTEGER,
        mode INTEGER,
        rssi INTEGER,
        free_heap INTEGER,
        uptime_s INTEGER
    )
    ''')
    cur.execute('CREATE INDEX IF NOT EXISTS idx_hist_mac ON telemetry_history(mac, timestamp)')
    conn.commit()
    conn.close()

HTML_DASHBOARD = '''<!DOCTYPE html>
<html lang="it">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Autodomain - Telemetria Termostati Smart</title>
    <style>
        :root {
            --bg: #0f172a;
            --card-bg: #1e293b;
            --border: #334155;
            --accent: #38bdf8;
            --accent-glow: rgba(56, 189, 248, 0.2);
            --text: #f8fafc;
            --text-muted: #94a3b8;
            --online: #22c55e;
            --offline: #64748b;
            --heating: #f97316;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Oxygen, Ubuntu, Cantarell, sans-serif;
            background: var(--bg);
            color: var(--text);
            padding: 24px;
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
            font-size: 1.6rem;
            font-weight: 700;
            background: linear-gradient(135deg, #38bdf8 0%, #818cf8 100%);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }
        .logo-title p {
            font-size: 0.85rem;
            color: var(--text-muted);
            margin-top: 4px;
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
        .kpi-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 16px;
            margin-bottom: 28px;
        }
        .kpi-card {
            background: var(--card-bg);
            border: 1px solid var(--border);
            border-radius: 12px;
            padding: 18px;
            position: relative;
            overflow: hidden;
        }
        .kpi-card::before {
            content: "";
            position: absolute;
            top: 0; left: 0; right: 0; height: 3px;
            background: var(--accent);
        }
        .kpi-title { font-size: 0.8rem; color: var(--text-muted); text-transform: uppercase; letter-spacing: 0.05em; }
        .kpi-value { font-size: 1.8rem; font-weight: 700; margin-top: 8px; }
        .kpi-sub { font-size: 0.75rem; color: var(--text-muted); margin-top: 4px; }
        
        .section-title {
            font-size: 1.1rem;
            font-weight: 600;
            margin-bottom: 14px;
            display: flex;
            align-items: center;
            gap: 8px;
        }
        .table-card {
            background: var(--card-bg);
            border: 1px solid var(--border);
            border-radius: 12px;
            overflow: hidden;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            text-align: left;
            font-size: 0.9rem;
        }
        th {
            background: rgba(15, 23, 42, 0.6);
            color: var(--text-muted);
            font-weight: 600;
            font-size: 0.75rem;
            text-transform: uppercase;
            letter-spacing: 0.05em;
            padding: 14px 16px;
            border-bottom: 1px solid var(--border);
        }
        td {
            padding: 14px 16px;
            border-bottom: 1px solid rgba(51, 65, 85, 0.5);
            vertical-align: middle;
        }
        tr:last-child td { border-bottom: none; }
        tr:hover td { background: rgba(255, 255, 255, 0.02); }
        .badge {
            display: inline-block;
            padding: 4px 10px;
            border-radius: 12px;
            font-size: 0.75rem;
            font-weight: 600;
        }
        .badge-fw { background: rgba(56, 189, 248, 0.15); color: #38bdf8; border: 1px solid rgba(56, 189, 248, 0.3); }
        .badge-online { background: rgba(34, 197, 94, 0.15); color: #4ade80; border: 1px solid rgba(34, 197, 94, 0.3); }
        .badge-offline { background: rgba(100, 116, 139, 0.15); color: #94a3b8; border: 1px solid rgba(100, 116, 139, 0.3); }
        .badge-heat-on { background: rgba(249, 115, 22, 0.2); color: #fb923c; border: 1px solid rgba(249, 115, 22, 0.4); animation: pulse 2s infinite; }
        .badge-heat-off { background: rgba(148, 163, 184, 0.1); color: #64748b; }
        @keyframes pulse { 0% { opacity: 0.7; } 50% { opacity: 1; } 100% { opacity: 0.7; } }
        .empty-state {
            padding: 40px;
            text-align: center;
            color: var(--text-muted);
        }
        .btn-hist {
            background: rgba(56, 189, 248, 0.1);
            color: var(--accent);
            border: 1px solid rgba(56, 189, 248, 0.3);
            border-radius: 6px;
            padding: 6px 12px;
            font-size: 0.8rem;
            cursor: pointer;
            transition: all 0.2s;
        }
        .btn-hist:hover {
            background: var(--accent);
            color: #0f172a;
        }
        .btn-del {
            background: rgba(239, 68, 68, 0.1);
            color: #ef4444;
            border: 1px solid rgba(239, 68, 68, 0.3);
            border-radius: 6px;
            padding: 6px 12px;
            font-size: 0.8rem;
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
        /* Modal */
        .modal {
            display: none;
            position: fixed;
            top: 0; left: 0; width: 100%; height: 100%;
            background: rgba(0, 0, 0, 0.7);
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
            <h1>Autodomain Telemetry Center</h1>
            <p>Monitoraggio Centralizzato Dispositivi Termostato Smart ESP32</p>
        </div>
        <div class="refresh-pill">
            <span style="display:inline-block; width:8px; height:8px; border-radius:50%; background:var(--online)"></span>
            Aggiornamento continuo &bull; <span id="clock">--:--:--</span>
        </div>
    </div>

    <div class="kpi-grid">
        <div class="kpi-card">
            <div class="kpi-title">Dispositivi nel Parco</div>
            <div class="kpi-value" id="kpi-total">0</div>
            <div class="kpi-sub">Totale censiti nel database</div>
        </div>
        <div class="kpi-card">
            <div class="kpi-title">Dispositivi Online</div>
            <div class="kpi-value" style="color:var(--online);" id="kpi-online">0</div>
            <div class="kpi-sub">Attivi negli ultimi 30 min</div>
        </div>
        <div class="kpi-card">
            <div class="kpi-title">Impianti in Riscaldamento</div>
            <div class="kpi-value" style="color:var(--heating);" id="kpi-heating">0</div>
            <div class="kpi-sub">Caldaie attualmente accese</div>
        </div>
        <div class="kpi-card">
            <div class="kpi-title">Ultima Versione Firmware</div>
            <div class="kpi-value" style="color:var(--accent);" id="kpi-version">v2.5.0</div>
            <div class="kpi-sub">Release corrente in produzione</div>
        </div>
    </div>

    <div class="section-title">
        <div>
            <span>Stato Termostati Clienti</span>
            <p style="font-size: 0.8rem; color: var(--text-muted); font-weight: normal; margin-top: 2px;">
                I dispositivi rilevati permangono indefinitamente in elenco (anche se offline). Possono essere rimossi singolarmente con il tasto Elimina.
            </p>
        </div>
    </div>

    <div class="table-card">
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
                    <th>Segnale WiFi</th>
                    <th>Ultimo Ping</th>
                    <th>Azioni</th>
                </tr>
            </thead>
            <tbody id="device-rows">
                <tr><td colspan="11" class="empty-state">Caricamento telemetria...</td></tr>
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
                            <th>Temperatura</th>
                            <th>Target</th>
                            <th>Umidit&agrave;</th>
                            <th>Caldaia</th>
                            <th>WiFi RSSI</th>
                            <th>Free Heap</th>
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

        async function loadDevices() {
            try {
                const res = await fetch('/api/devices');
                const data = await res.json();
                
                document.getElementById('kpi-total').innerText = data.total;
                document.getElementById('kpi-online').innerText = data.online;
                document.getElementById('kpi-heating').innerText = data.heating_count;
                
                const tbody = document.getElementById('device-rows');
                if (!data.devices || data.devices.length === 0) {
                    tbody.innerHTML = `<tr><td colspan="11" class="empty-state">In attesa del primo segnale di telemetria dai dispositivi in campo...<br><small style="color:#64748b;">(I termostati inviano un report automatico ogni 15 minuti via HTTP a questo server)</small></td></tr>`;
                    return;
                }

                tbody.innerHTML = data.devices.map(d => {
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
            } catch(e) {
                console.error("Fetch error:", e);
            }
        }

        async function deleteDevice(mac) {
            if (!confirm("Sei sicuro di voler eliminare definitivamente il dispositivo " + mac + " dal database?\nL'operazione rimuoverà la scheda e tutto il relativo storico.")) {
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
                        <td><strong>${h.temp != null ? h.temp.toFixed(1) + '°C' : '-'}</strong></td>
                        <td>${h.target_temp != null ? h.target_temp.toFixed(1) + '°C' : '-'}</td>
                        <td>${h.humidity != null ? h.humidity.toFixed(0) + '%' : '-'}</td>
                        <td>${h.heating ? '<span style="color:#f97316">ACCESA</span>' : 'SPENTA'}</td>
                        <td>${h.rssi || '-'} dBm</td>
                        <td>${h.free_heap ? (h.free_heap/1024).toFixed(0) + ' KB' : '-'}</td>
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

class TelemetryHandler(BaseHTTPRequestHandler):
    def _send_json(self, data, code=200):
        body = json.dumps(data).encode('utf-8')
        self.send_response(code)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, DELETE, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.send_header('Content-Length', str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_OPTIONS(self):
        self.send_response(204)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, DELETE, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()

    def do_GET(self):
        parsed = urlparse(self.path)
        path = parsed.path
        
        if path == "/" or path == "/dashboard":
            body = HTML_DASHBOARD.encode('utf-8')
            self.send_response(200)
            self.send_header('Content-Type', 'text/html; charset=utf-8')
            self.send_header('Content-Length', str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/api/devices":
            conn = sqlite3.connect(DB_PATH)
            conn.row_factory = sqlite3.Row
            cur = conn.cursor()
            cur.execute("SELECT * FROM devices ORDER BY last_seen DESC")
            rows = [dict(r) for r in cur.fetchall()]
            conn.close()

            now = datetime.datetime.now(datetime.timezone.utc)
            total = len(rows)
            online_count = 0
            heating_count = 0

            for r in rows:
                is_online = False
                if r.get("last_seen"):
                    try:
                        ls = datetime.datetime.strptime(r["last_seen"], "%Y-%m-%d %H:%M:%S").replace(tzinfo=datetime.timezone.utc)
                        if (now - ls).total_seconds() < 1800: # 30 min
                            is_online = True
                    except Exception as ex:
                        pass
                r["is_online"] = is_online
                if is_online:
                    online_count += 1
                if r.get("last_heating"):
                    heating_count += 1

            self._send_json({
                "total": total,
                "online": online_count,
                "heating_count": heating_count,
                "devices": rows
            })
            return

        if path == "/api/history":
            qs = parse_qs(parsed.query)
            mac = qs.get("mac", [""])[0]
            limit = int(qs.get("limit", [50])[0])
            
            conn = sqlite3.connect(DB_PATH)
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

            conn = sqlite3.connect(DB_PATH)
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
        
        # Endpoint per cancellazione singola dispositivo
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

            conn = sqlite3.connect(DB_PATH)
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

        mac = data.get("mac")
        if not mac:
            self._send_json({"error": "Missing mac field"}, 400)
            return

        now_str = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%d %H:%M:%S")
        client_ip = self.client_address[0]
        model = data.get("model", "cyd")
        version = data.get("version", "2.5.0")
        temp = data.get("temp")
        target_temp = data.get("target_temp")
        humidity = data.get("humidity")
        heating = 1 if data.get("heating") else 0
        mode = data.get("mode", 0)
        rssi = data.get("rssi", 0)
        uptime = data.get("uptime_s", 0)
        free_heap = data.get("free_heap", 0)

        conn = sqlite3.connect(DB_PATH)
        cur = conn.cursor()

        # Update or insert device
        cur.execute('''
        INSERT INTO devices (mac, model, version, last_ip, first_seen, last_seen, last_temp, last_target, last_humidity, last_heating, last_rssi, last_uptime, last_heap, total_packets)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, 1)
        ON CONFLICT(mac) DO UPDATE SET
            model = excluded.model,
            version = excluded.version,
            last_ip = excluded.last_ip,
            last_seen = excluded.last_seen,
            last_temp = excluded.last_temp,
            last_target = excluded.last_target,
            last_humidity = excluded.last_humidity,
            last_heating = excluded.last_heating,
            last_rssi = excluded.last_rssi,
            last_uptime = excluded.last_uptime,
            last_heap = excluded.last_heap,
            total_packets = devices.total_packets + 1
        ''', (mac, model, version, client_ip, now_str, now_str, temp, target_temp, humidity, heating, rssi, uptime, free_heap))

        # Insert history record
        cur.execute('''
        INSERT INTO telemetry_history (mac, timestamp, temp, target_temp, humidity, heating, mode, rssi, free_heap, uptime_s)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        ''', (mac, now_str, temp, target_temp, humidity, heating, mode, rssi, free_heap, uptime))

        conn.commit()
        conn.close()

        self._send_json({"status": "ok", "received_at": now_str})

def main():
    init_db()
    server = HTTPServer(('0.0.0.0', PORT), TelemetryHandler)
    print(f"Telemetry server running on port {PORT}...")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    server.server_close()

if __name__ == '__main__':
    main()
