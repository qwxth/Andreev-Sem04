from pydantic_settings import BaseSettings
from functools import lru_cache

class Settings(BaseSettings):
    opensearch_host: str = "https://opensearch:9200"
    opensearch_user: str = "admin"
    opensearch_password: str = "Admin_password123!"
    index_name: str = "ia_index1"
    ollama_host: str = "http://ollama:11434"
    embedding_model: str = "qwen3-embedding"
    generation_model: str = "qwen3"
    top_k_results: int = 5
    top_m_context: int = 3
    embedding_dimension: int = 4096

    class Config:
        env_file = ".env"
        case_sensitive = False

@lru_cache()
def get_settings() -> Settings:
    return Settings()
