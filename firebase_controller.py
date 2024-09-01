import firebase_admin
from firebase_admin import credentials, db
import logging

logging.basicConfig(level=logging.DEBUG)

class FirebaseSingleton:
    _instance = None

    def __new__(cls, *args, **kwargs):
        if cls._instance is None:
            cls._instance = super(FirebaseSingleton, cls).__new__(cls, *args, **kwargs)
            # Initialize Firebase only once
            cls._initialize_firebase()
        return cls._instance

    @classmethod
    def _initialize_firebase(cls):
        if not firebase_admin._apps:
            logging.debug("Initializing Firebase...")
            cred = credentials.Certificate(r"C:\Users\User\Documents\Kuliah\sems3\belajar-web\absens-39dd0-firebase-adminsdk-fy2w3-965e91c79c.json")
            firebase_admin.initialize_app(cred, {
                'databaseURL': 'https://absens-39dd0-default-rtdb.asia-southeast1.firebasedatabase.app'
            })
        else:
            logging.debug("Firebase is already initialized.")

    @staticmethod
    def get_db_reference(path: str):
        """Get a reference to a specific path in the Firebase database."""
        return db.reference(path)
    
    @staticmethod
    def add_listener(path: str, callback):
        """Add a listener to a specific path in the Firebase database."""
        ref = FirebaseSingleton.get_db_reference(path)
        ref.listen(callback)
    