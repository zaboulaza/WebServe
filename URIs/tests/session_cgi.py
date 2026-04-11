#!/usr/bin/env python3
"""
Démo bonus : Cookie + Session via CGI Python
Le serveur injecte SESSION_ID et SESSION_COUNT dans l'environnement.
Le script affiche ces infos et génère sa propre réponse HTML.
"""
import os

session_id    = os.environ.get("SESSION_ID", "inconnue")
session_count = os.environ.get("SESSION_COUNT", "0")
http_cookie   = os.environ.get("HTTP_COOKIE", "(aucun cookie reçu)")
method        = os.environ.get("REQUEST_METHOD", "GET")

print("Content-Type: text/html")
print("")
print(f"""<!DOCTYPE html>
<html><head><title>Session CGI Demo</title>
<style>
  body {{ font-family: monospace; padding: 20px; }}
  table {{ border-collapse: collapse; margin: 10px 0; }}
  td, th {{ border: 1px solid #999; padding: 8px 12px; }}
  .sid {{ color: #0066cc; font-weight: bold; }}
</style>
</head><body>
<h1>Demo Bonus — Sessions via CGI Python</h1>
<h2>Données de session (injectées par le serveur)</h2>
<table>
  <tr><th>Variable</th><th>Valeur</th></tr>
  <tr><td>SESSION_ID</td><td class="sid">{session_id}</td></tr>
  <tr><td>SESSION_COUNT</td><td>{session_count}</td></tr>
  <tr><td>HTTP_COOKIE</td><td>{http_cookie}</td></tr>
  <tr><td>REQUEST_METHOD</td><td>{method}</td></tr>
</table>
<p>
  <a href="/tests/session_cgi.py">Recharger (incrémenter le compteur)</a> |
  <a href="/session">Page session serveur</a> |
  <a href="/">Accueil</a>
</p>
<p style="color:gray;font-size:0.8em">
  Le cookie <code>session_id</code> est positionné automatiquement
  par le serveur lors de la première requête (Set-Cookie).
</p>
</body></html>""")
