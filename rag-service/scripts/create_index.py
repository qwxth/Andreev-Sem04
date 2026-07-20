#!/usr/bin/env python3
import argparse
import sys
import json
import logging
from opensearchpy import OpenSearch, RequestsHttpConnection
from opensearchpy.exceptions import RequestError

logging.basicConfig(level=logging.INFO, format="%(asctime)s - %(levelname)s - %(message)s")
logger = logging.getLogger(__name__)

DEFAULT_HOST = "https://localhost:9200"
DEFAULT_USER = "admin"
DEFAULT_PASSWORD = "Admin_password123!"
DEFAULT_INDEX = "ia_index1"
EMBEDDING_DIMENSION = 4096  # qwen3-embedding

INDEX_BODY = {
    "settings": {
        "index": {
            "number_of_shards": 1,
            "number_of_replicas": 1,
            "knn": True,
            "knn.algo_param.ef_search": 512
        }
    },
    "mappings": {
        "properties": {
            "content": {
                "type": "text"
            },
            "embedding": {
                "type": "knn_vector",
                "dimension": EMBEDDING_DIMENSION,
                "method": {
                    "name": "hnsw",
                    "space_type": "innerproduct",
                    "engine": "lucene",
                    "parameters": {
                        "m": 16,
                        "ef_construction": 512
                    }
                }
            },
            "chunkIndex":              {"type": "integer"},
            "classifiers":             {"type": "keyword"},
            "cover_letter_probability":{"type": "double"},
            "docclass":                {"type": "text"},
            "documentDate":            {"type": "date"},
            "documentId":              {"type": "keyword"},
            "fileId":                  {"type": "keyword"},
            "is_cover_letter":         {"type": "boolean"},
            "model":                   {"type": "keyword"},
            "note":                    {"type": "text"},
            "real_dirid":              {"type": "keyword"},
            "real_dirname":            {"type": "text"},
            "resolution_project":      {"type": "text"},
            "responsible_executor":    {"type": "keyword"},
            "senderfull":              {"type": "text"},
            "summary":                 {"type": "text"},
            "coexecutor_persons": {
                "type": "nested",
                "properties": {
                    "fio": {"type": "text", "fields": {"keyword": {"type": "keyword"}}},
                    "id":  {"type": "keyword"}
                }
            },
            "coexecutor_subdivision": {
                "type": "nested",
                "properties": {
                    "dirid":   {"type": "keyword"},
                    "dirname": {"type": "text", "fields": {"keyword": {"type": "keyword"}}}
                }
            },
            "know": {
                "type": "nested",
                "properties": {
                    "fio": {"type": "text", "fields": {"keyword": {"type": "keyword"}}},
                    "id":  {"type": "keyword"}
                }
            }
        }
    }
}

def get_client(host, user, password):
    return OpenSearch(
        hosts=[host],
        http_auth=(user, password),
        use_ssl=True,
        verify_certs=False,
        ssl_show_warn=False,
        connection_class=RequestsHttpConnection,
        timeout=60
    )

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--host",     default=DEFAULT_HOST)
    parser.add_argument("--user",     default=DEFAULT_USER)
    parser.add_argument("--password", default=DEFAULT_PASSWORD)
    parser.add_argument("--index",    default=DEFAULT_INDEX)
    parser.add_argument("--recreate", action="store_true")
    args = parser.parse_args()

    client = get_client(args.host, args.user, args.password)

    try:
        health = client.cluster.health()
        logger.info(f"OpenSearch: {health.get('status')}")
    except Exception as e:
        logger.error(f"OpenSearch недоступен: {e}")
        sys.exit(1)

    if args.recreate and client.indices.exists(index=args.index):
        client.indices.delete(index=args.index)
        logger.info(f"Индекс '{args.index}' удалён")

    if client.indices.exists(index=args.index):
        logger.warning(f"Индекс '{args.index}' уже существует")
        # Показать текущую размерность
        mapping = client.indices.get_mapping(index=args.index)
        dim = mapping[args.index]['mappings']['properties']['embedding'].get('dimension')
        logger.info(f"Текущая размерность embedding: {dim}")
        sys.exit(0)

    try:
        response = client.indices.create(index=args.index, body=INDEX_BODY)
        if response.get("acknowledged"):
            logger.info(f"✓ Индекс '{args.index}' создан")
            logger.info(f"  embedding dimension: {EMBEDDING_DIMENSION} (qwen3-embedding)")
            logger.info(f"  Алгоритм: HNSW innerproduct")
        else:
            logger.error(f"Ошибка: {response}")
            sys.exit(1)
    except Exception as e:
        logger.error(f"Ошибка создания: {e}")
        sys.exit(1)

    logger.info("Готово!")

if __name__ == "__main__":
    main()
