pipeline {
  agent { label 'MacMini' }
  options { skipDefaultCheckout() }
  parameters {
    gitParameter(name: 'GIT_TAG',
                 type: 'PT_BRANCH_TAG',
                 description: 'The Git tag to checkout. If not specified "master" will be checkout.',
                 defaultValue: 'master')
    booleanParam(name: 'BUILD_MACOSX',
                 description: 'Build macosx target.',
                 defaultValue: true)
    booleanParam(name: 'BUILD_LINUX64',
                 description: 'Build x86_64-unknown-linux-gnu target.',
                 defaultValue: true)
    booleanParam(name: 'BUILD_LINUX32',
                 description: 'Build i686-unknown-linux-gnu target.',
                 defaultValue: true)
    booleanParam(name: 'BUILD_WIN64',
                 description: 'Build x86_64-pc-windows-gnu target.',
                 defaultValue: true)
    booleanParam(name: 'PUBLISH_ECLIPSE_DOWNLOAD',
                 description: 'Publish the resulting artifacts (to Eclipse download)',
                 defaultValue: false)
  }
  environment {
      LABEL = get_label()
      MACOSX_DEPLOYMENT_TARGET=10.7
  }

  stages {
    stage('[MacMini] Checkout Git TAG') {
      steps {
        deleteDir()
        checkout([$class: 'GitSCM',
                  branches: [[name: "${params.GIT_TAG}"]],
                  doGenerateSubmoduleConfigurations: false,
                  extensions: [],
                  gitTool: 'Default',
                  submoduleCfg: [],
                  userRemoteConfigs: [[url: 'https://github.com/eclipse-zenoh/zenoh-c.git']]
                ])
      }
    }

    stage('[MacMini] MacOS Build') {
      when { expression { return params.BUILD_MACOSX }}
      steps {
        sh '''
        env
        make all
        '''
      }
    }
    stage('[MacMini] MacOS Package') {
      when { expression { return params.BUILD_MACOSX }}
      steps {
        sh '''
        tar -cvf eclipse-zenoh-c-${LABEL}-macosx${MACOSX_DEPLOYMENT_TARGET}-x86-64.tar --strip-components 2 target/release/*.dylib
        tar -rvf eclipse-zenoh-c-${LABEL}-macosx${MACOSX_DEPLOYMENT_TARGET}-x86-64.tar include
        gzip eclipse-zenoh-c-${LABEL}-macosx${MACOSX_DEPLOYMENT_TARGET}-x86-64.tar
        tar -czvf eclipse-zenoh-c-${LABEL}-examples-macosx${MACOSX_DEPLOYMENT_TARGET}-x86-64.tar.gz --exclude 'target/release/examples/*.*' --strip-components 3 target/release/examples/*
        '''
      }
    }

    stage('[MacMini] x86_64-unknown-linux-gnu Build') {
      when { expression { return params.BUILD_LINUX64 }}
      steps {
        sh '''
        docker run --init --rm -v $(pwd):/workdir -w /workdir --env "TARGET=x86_64-unknown-linux-gnu" adlinktech/manylinux2010-x64-rust-nightly \
          /bin/bash -c "\
            make all && \
            cargo deb --target=x86_64-unknown-linux-gnu --variant=libzenohc && \
            cargo deb --target=x86_64-unknown-linux-gnu --variant=libzenohc-dev
          "
        '''
      }
    }
    stage('[MacMini] x86_64-unknown-linux-gnu Package') {
      when { expression { return params.BUILD_LINUX64 }}
      steps {
        sh '''
        tar -cvf eclipse-zenoh-c-${LABEL}-x86_64-unknown-linux-gnu.tar --strip-components 3 target/x86_64-unknown-linux-gnu/release/*.so
        tar -rvf eclipse-zenoh-c-${LABEL}-x86_64-unknown-linux-gnu.tar include
        gzip eclipse-zenoh-c-${LABEL}-x86_64-unknown-linux-gnu.tar
        tar -czvf eclipse-zenoh-c-${LABEL}-examples-x86_64-unknown-linux-gnu.tar.gz --exclude 'target/x86_64-unknown-linux-gnu/release/examples/*.*' --exclude 'target/x86_64-unknown-linux-gnu/release/examples/*-*' --strip-components 4 target/x86_64-unknown-linux-gnu/release/examples/*
        '''
      }
    }

    stage('[MacMini] i686-unknown-linux-gnu Build') {
      when { expression { return params.BUILD_LINUX32 }}
      steps {
        sh '''
        docker run --init --rm -v $(pwd):/workdir -w /workdir --env "TARGET=i686-unknown-linux-gnu" adlinktech/manylinux2010-i686-rust-nightly \
          make all && \
            cargo deb --target=i686-unknown-linux-gnu --variant=libzenohc && \
            cargo deb --target=i686-unknown-linux-gnu --variant=libzenohc-dev
        '''
      }
    }
    stage('[MacMini] i686-unknown-linux-gnu Package') {
      when { expression { return params.BUILD_LINUX32 }}
      steps {
        sh '''
        tar -cvf eclipse-zenoh-c-${LABEL}-i686-unknown-linux-gnu.tar --strip-components 3 target/i686-unknown-linux-gnu/release/*.so
        tar -rvf eclipse-zenoh-c-${LABEL}-i686-unknown-linux-gnu.tar include
        gzip eclipse-zenoh-c-${LABEL}-i686-unknown-linux-gnu.tar
        tar -czvf eclipse-zenoh-c-${LABEL}-examples-i686-unknown-linux-gnu.tar.gz --exclude 'target/i686-unknown-linux-gnu/release/examples/*.*' --exclude 'target/i686-unknown-linux-gnu/release/examples/*-*' --strip-components 4 target/i686-unknown-linux-gnu/release/examples/*
        '''
      }
    }

    stage('[MacMini] x86_64-pc-windows-gnu Build') {
      when { expression { return params.BUILD_WIN64 }}
      steps {
        sh '''
        cbindgen --config cbindgen.toml --crate zenoh-c --output include/zenoh/net.h
        cargo rustc --verbose --release --bins --lib --examples --target=x86_64-pc-windows-gnu -- -C panic=abort
        '''
      }
    }
    stage('[MacMini] x86_64-pc-windows-gnu Package') {
      when { expression { return params.BUILD_WIN64 }}
      steps {
        sh '''
        zip eclipse-zenoh-c-${LABEL}-x86_64-pc-windows-gnu.zip --junk-paths target/x86_64-pc-windows-gnu/release/zenohc.dll
        zip eclipse-zenoh-c-${LABEL}-x86_64-pc-windows-gnu.zip -r -u include
        '''
      }
    }

    stage('[MacMini] Publish to download.eclipse.org') {
      when { expression { return params.PUBLISH_ECLIPSE_DOWNLOAD }}
      steps {
        sshagent ( ['projects-storage.eclipse.org-bot-ssh']) {
          sh '''
          export TARGET_DIR=/home/data/httpd/download.eclipse.org/zenoh/zenoh-c/${LABEL}/
          ssh genie.zenoh@projects-storage.eclipse.org mkdir -p ${TARGET_DIR}
          if [ "${BUILD_MACOSX}" = "true" ]; then
            scp eclipse-zenoh-c-${LABEL}-*macosx*.tar.gz genie.zenoh@projects-storage.eclipse.org:${TARGET_DIR}
          fi
          if [ "${BUILD_LINUX64}" = "true" ]; then
            scp eclipse-zenoh-c-${LABEL}-*x86_64-unknown-linux-gnu.tar.gz target/x86_64-unknown-linux-gnu/debian/*.deb genie.zenoh@projects-storage.eclipse.org:${TARGET_DIR}
            scp target/x86_64-unknown-linux-gnu/debian/*.deb genie.zenoh@projects-storage.eclipse.org:/home/data/httpd/download.eclipse.org/zenoh/zenoh/${LABEL}/
          fi
          if [ "${BUILD_LINUX32}" = "true" ]; then
            scp eclipse-zenoh-c-${LABEL}-*i686-unknown-linux-gnu.tar.gz target/i686-unknown-linux-gnu/debian/*.deb genie.zenoh@projects-storage.eclipse.org:${TARGET_DIR}
            scp target/x86_64-unknown-linux-gnu/debian/*.deb genie.zenoh@projects-storage.eclipse.org:/home/data/httpd/download.eclipse.org/zenoh/zenoh/${LABEL}/
          fi
          if [ "${BUILD_WIN64}" = "true" ]; then
            scp eclipse-zenoh-c-${LABEL}-*x86_64-pc-windows-gnu.zip genie.zenoh@projects-storage.eclipse.org:${TARGET_DIR}
          fi
          '''
        }
      }
    }

    stage('[UbuntuVM] Build Packages.gz for download.eclipse.org') {
      agent { label 'UbuntuVM' }
      when { expression { return params.PUBLISH_ECLIPSE_DOWNLOAD && (params.BUILD_LINUX64 || params.BUILD_LINUX32) }}
      steps {
        deleteDir()
        sshagent ( ['projects-storage.eclipse.org-bot-ssh']) {
          sh '''
          scp genie.zenoh@projects-storage.eclipse.org:/home/data/httpd/download.eclipse.org/zenoh/zenoh/${LABEL}/*.deb ./
          dpkg-scanpackages --multiversion . > Packages
          cat Packages
          gzip -c9 < Packages > Packages.gz
          scp Packages.gz genie.zenoh@projects-storage.eclipse.org:/home/data/httpd/download.eclipse.org/zenoh/zenoh/${LABEL}/
          '''
        }
      }
    }
  }
}

def get_label() {
    return env.GIT_TAG.startsWith('origin/') ? env.GIT_TAG.minus('origin/') : env.GIT_TAG
}