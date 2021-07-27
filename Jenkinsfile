pipeline {
  agent any
  stages {
    stage('TEST') {
      parallel {
        stage('TEST') {
          steps {
            echo 'Hello'
            echo "${sosa}"
          }
        }

        stage('Test2') {
          steps {
            sh '''sleep 10
echo done'''
          }
        }

      }
    }

  }
  environment {
    sosa = 'lity'
  }
}