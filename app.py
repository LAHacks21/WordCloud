from flask import Flask, request, send_from_directory, render_template, jsonify
from flask_cors import CORS
from wordc import wordc
app = Flask(__name__, static_url_path='')
CORS(app)

@app.route('/')
def index():
    return render_template("wordcloud.html")

@app.route('/cloud', methods=['GET'])
def cloud():
    if request.method == 'GET':
        print(request.args, request.args['topic'])
        try:
            user_max = int(request.args['numWords'])
        except ValueError:
            user_max = 80
        color = bool(request.args['color'])
        wordc(name=request.args['topic'], 
                topic=request.args['topic'], 
                user_max=user_max,
                color=color)
        return f"img/{request.args['topic']}{user_max}{color}.jpg"
# def root():
#     return app.send_static_file('wordcloud.html')

# @app.route('/<path:path>')
# def send_path(path):
#     return app.send_static_file(path)

if __name__ == '__main__':
    app.run()

