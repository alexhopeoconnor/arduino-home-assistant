FROM python:3.12-slim

WORKDIR /tests
COPY test-support/ha-mqtt-contract /opt/ha-mqtt-contract
COPY tests/ha-contract/requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt
RUN pip install --no-cache-dir /opt/ha-mqtt-contract
COPY tests/ha-contract/test_contract.py .

CMD ["python", "test_contract.py"]
